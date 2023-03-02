#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <cassert>
#include <string_view>

// ====================================================================
// 24 bit color
class Color {
public:
  Color(std::uint8_t r_=255, std::uint8_t g_=255, std::uint8_t b_=255) : r(r_),g(g_),b(b_) {}
  [[nodiscard]] bool isWhite() const { return r==255 && g==255 && b==255; }
  std::uint8_t r,g,b;
};

// ====================================================================
// ====================================================================
// save and load from the .ppm image file format (not fully compliant)

class Image {
public:
  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Image(std::string_view filename = ""): width{}, height{}, data{}
  {
    if (filename != "") Load(filename); 
  }
  Image(int w, int h): width{w}, height{h}, data{new Color[width*height]} {}
  void Allocate(int w, int h) {
    width = w;
    height = h;
    delete [] data;
    if (width == 0 && height == 0) {
      data = nullptr;
    } else {
      assert (width > 0 && height > 0);
      data = new Color[width*height]; 
    }
  }
  ~Image() {
    delete [] data;
  }

  Image(const Image &image) { 
    copy_helper(image); }
  const Image& operator=(const Image &image) { 
    if (this != &image)
      copy_helper(image);
    return *this; }

  void copy_helper(const Image &image) {
    Allocate(image.Width(), image.Height());
    for (int i = 0; i < image.Width(); i++) {
      for (int j = 0; j < image.Height(); j++) {
        this->SetPixel(i,j,image.GetPixel(i,j));
      }
    }
  }

  // =========
  // ACCESSORS
  [[nodiscard]] int Width() const { return width; }
  [[nodiscard]] int Height() const { return height; }
  [[nodiscard]] const Color& GetPixel(int x, int y) const {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    return data[y*width + x]; }

  // =========
  // MODIFIERS
  void SetAllPixels(const Color &value) {
    for (int i = 0; i < width*height; i++) {
      data[i] = value; } }
  void SetPixel(int x, int y, const Color &value) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    data[y*width + x] = value; }

  // ===========
  // LOAD & SAVE
  bool Load(std::string_view filename);
  bool Save(std::string_view filename) const;

private:
  // ==============
  // REPRESENTATION
  int width;
  int height;
  Color *data;
};

#endif
