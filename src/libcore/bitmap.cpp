#include <core/bitmap.h>
#include <fstream>
#include <core/utils.h>
#include <core/logger.h>

#define TINYEXR_USE_MINIZ 0
#include <core/miniz.h>
#define TINYEXR_IMPLEMENTATION
#include <core/tinyexr.h>

Bitmap::Bitmap(const ArrayXd &data, const Vector2i &res)
{
    m_data = from_tensor_to_spectrum_list(
        data, res.prod());
    m_res = res;
}

void Bitmap::fill(const Spectrum &value)
{
    m_res = Vector2i(1, 1);
    m_data.resize(1);
    m_data[0] = value;
}

void Bitmap::setZero()
{
    for (auto &v : m_data)
        v.setZero();
}

void Bitmap::load(const char *filename)
{
    float *out; // width * height * RGBA
    int width;
    int height;
    const char *err = NULL; // or nullptr in C++11

    int ret = LoadEXR(&out, &width, &height, filename, &err);
    int size = width * height;

    if (ret != TINYEXR_SUCCESS)
    {
        if (err)
        {
            fprintf(stderr, "ERR : %s\n", err);
            FreeEXRErrorMessage(err); // release memory of error message.
        }
    }

    m_data.resize(size);
    int offset = 0;
    for (int i = 0; i < height; ++i)
        for (int j = 0; j < width; ++j)
        {
            m_data[offset] = Spectrum(out[offset * 4 + 0],
                                      out[offset * 4 + 1],
                                      out[offset * 4 + 2]);
            ++offset;
        }
    free(out); // release memory of image data
    m_res = Vector2i(width, height);
}

void Bitmap::save(const char *filename) const {
    int width = m_res.x();
    int height = m_res.y();
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    std::vector<float> images[3];
    images[0].resize(width * height);
    images[1].resize(width * height);
    images[2].resize(width * height);

    for (int i = 0; i < width * height; i++) {
      images[0][i] = m_data[i].x();
      images[1][i] = m_data[i].y();
      images[2][i] = m_data[i].z();
    }

    float* image_ptr[3];
    image_ptr[0] = &(images[2].at(0)); // B
    image_ptr[1] = &(images[1].at(0)); // G
    image_ptr[2] = &(images[0].at(0)); // R

    image.images = (unsigned char**)image_ptr;
    image.width = width;
    image.height = height;

    header.num_channels = 3;
    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
    strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
    strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
      header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    const char* err = NULL; // or nullptr in C++11 or later.
    int ret = SaveEXRImageToFile(&image, &header, filename, &err);
    if (ret != TINYEXR_SUCCESS) {
      PSDR_ERROR("Save EXR err: %s\n", err);
      FreeEXRErrorMessage(err); // free's buffer for an error message
      return;
    }
    PSDR_INFO("Saved exr file. {} \n", filename);

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);
}

Spectrum Bitmap::eval(const Vector2 &_uv, bool flip_v) const
{
    if (m_res.x() == 1 && m_res.y() == 1)
        return m_data[0];
    Array2 uv = _uv.array();
    if (flip_v)
        uv.y() = -uv.y();
    uv -= uv.floor();
    uv *= m_res.cast<Float>().array() - Array2(1, 1);
    Array2i pos = uv.floor().cast<int>();
    Array2 w1 = uv - pos.cast<Float>();
    Array2 w0 = Array2(1, 1) - w1;
    pos = pos.min(m_res.array() - Array2i(2, 2));
    int index = pos.y() * m_res.x() + pos.x();
    Spectrum v00 = m_data[index],
             v10 = m_data[index + 1],
             v01 = m_data[index + m_res.x()],
             v11 = m_data[index + m_res.x() + 1];

    Spectrum v0 = v00 * w0.x() + v10 * w1.x(),
             v1 = v01 * w0.x() + v11 * w1.x();
    Spectrum ret = w0.y() * v0 + w1.y() * v1;
    assert(ret.allFinite());
    return ret;
}

ArrayXd Bitmap::getData() const
{
    return from_spectrum_list_to_tensor(m_data, m_res.prod());
}

void Bitmap::setData(const ArrayXd &data)
{
    m_data = from_tensor_to_spectrum_list(data, m_res.prod());
}

std::string Bitmap::toString() const
{
    std::stringstream ss;
    ss << "Bitmap[" << m_res << "]";
    return ss.str();
}
