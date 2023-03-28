#ifndef _VECTORS_H_
#define _VECTORS_H_

//
// originally implemented by Justin Legakis
// modified by Mingyi Chen, Feb 2023
//

#include <iostream>
#include <cassert>
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Matrix;

// ====================================================================
// ====================================================================

class Vec3f {

public:

  // -----------------------------------------------
  // CONSTRUCTORS, ASSIGNMENT OPERATOR, & DESTRUCTOR
  constexpr Vec3f() noexcept : data{} {}
  constexpr Vec3f(const Vec3f &V) noexcept : data{V.data[0], V.data[1], V.data[2]} {}
  constexpr Vec3f(double d0, double d1, double d2) noexcept : data{d0, d1, d2} {}
  constexpr Vec3f& operator=(const Vec3f &V) noexcept {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
    return *this; }

  // ----------------------------
  // SIMPLE ACCESSORS & MODIFIERS
  [[nodiscard]] double operator[](int i) const { 
    assert (i >= 0 && i < 3); 
    return data[i]; }
  [[nodiscard]] constexpr double x() const noexcept { return data[0]; }
  [[nodiscard]] constexpr double y() const noexcept { return data[1]; }
  [[nodiscard]] constexpr double z() const noexcept { return data[2]; }
  [[nodiscard]] constexpr double r() const noexcept { return data[0]; }
  [[nodiscard]] constexpr double g() const noexcept { return data[1]; }
  [[nodiscard]] constexpr double b() const noexcept { return data[2]; }
  constexpr void setx(double x) noexcept { data[0]=x; }
  constexpr void sety(double y) noexcept { data[1]=y; }
  constexpr void setz(double z) noexcept { data[2]=z; }
  constexpr void set(double d0, double d1, double d2) noexcept {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2; }

  // ----------------
  // EQUALITY TESTING 
  constexpr bool operator==(const Vec3f &V) const noexcept {
    return data[0] == V.data[0] &&
	    data[1] == V.data[1] &&
	    data[2] == V.data[2]; }
  constexpr bool operator!=(const Vec3f &V) const noexcept {
    return data[0] != V.data[0] ||
	    data[1] != V.data[1] ||
	    data[2] != V.data[2]; }

  // ------------------------
  // COMMON VECTOR OPERATIONS
  [[nodiscard]] constexpr double Length() const noexcept {
    return sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]); }
  constexpr void Normalize() noexcept {
    if (auto len{Length()}) (*this) /= len; }
  [[nodiscard]] constexpr Vec3f Normalized() const noexcept {
    if (auto len{Length()}) return {data[0] / len, data[1] / len, data[2] / len};
    return *this; }
  constexpr void Negate() noexcept { (*this) *= -1; }
  [[nodiscard]] constexpr double Dot3(const Vec3f &V) const noexcept {
    return data[0] * V.data[0] +
      data[1] * V.data[1] +
      data[2] * V.data[2] ; }
  static void Cross3(Vec3f &c, const Vec3f &v1, const Vec3f &v2) noexcept {
    double x = v1.data[1]*v2.data[2] - v1.data[2]*v2.data[1];
    double y = v1.data[2]*v2.data[0] - v1.data[0]*v2.data[2];
    double z = v1.data[0]*v2.data[1] - v1.data[1]*v2.data[0];
    c.data[0] = x; c.data[1] = y; c.data[2] = z; }

  // ---------------------
  // VECTOR MATH OPERATORS
  constexpr Vec3f& operator+=(const Vec3f &V) noexcept {
    data[0] += V.data[0];
    data[1] += V.data[1];
    data[2] += V.data[2];
    return *this; }
  constexpr Vec3f& operator-=(const Vec3f &V) noexcept {
    data[0] -= V.data[0];
    data[1] -= V.data[1];
    data[2] -= V.data[2];
    return *this; }
  constexpr Vec3f& operator*=(double d) noexcept {
    data[0] *= d;
    data[1] *= d;
    data[2] *= d;
    return *this; }
  constexpr Vec3f& operator/=(double d) noexcept {
    data[0] /= d;
    data[1] /= d;
    data[2] /= d;
    return *this; }  
  friend constexpr Vec3f operator+(const Vec3f &v1, const Vec3f &v2) noexcept { 
    Vec3f v3 = v1; v3 += v2; return v3; }
  friend constexpr Vec3f operator-(const Vec3f &v1) {
    return v1 * -1; }
  friend constexpr Vec3f operator-(const Vec3f &v1, const Vec3f &v2) noexcept {
    return Vec3f{v1} -= v2; }
  friend constexpr Vec3f operator*(const Vec3f &v1, double d) noexcept {
    return {v1.data[0] * d, v1.data[1] * d, v1.data[2] * d}; }
  friend constexpr Vec3f operator*(const Vec3f &v1, const Vec3f &v2) noexcept {
    return {v1.data[0] * v2.data[0], v1.data[1] * v2.data[1], v1.data[2] * v2.data[2]}; }
  friend constexpr Vec3f operator*(double d, const Vec3f &v1) noexcept {
    return v1 * d; }

  // --------------
  // INPUT / OUTPUT
  friend std::ostream& operator<<(std::ostream &ostr, const Vec3f &v) {
    ostr << "< " << v.data[0] << " , " << v.data[1] << " , " << v.data[2] << " >";
    return ostr; }
  friend std::istream& operator>>(std::istream &istr, Vec3f &v) {
    char ch_a,ch_b,ch_c,ch_d;
    istr >> ch_a >> v.data[0] >> ch_b >> v.data[1] >> ch_c >> v.data[2] >> ch_d;
    assert (ch_a == '<');
    assert (ch_b == ',');
    assert (ch_c == ',');
    assert (ch_d == '>');
    return istr; }

private:

  friend class Matrix;

  // REPRESENTATION
  double data[3];
  
};

// ====================================================================
// ====================================================================

class Vec4f {
  
public:

  // CONSTRUCTORS, ASSIGNMENT OPERATOR, & DESTRUCTOR
  constexpr Vec4f() noexcept : data{} {}
  constexpr Vec4f(const Vec4f &V) noexcept :
    data{V.data[0], V.data[1], V.data[2], V.data[3]} {}
  constexpr Vec4f(double d0, double d1, double d2, double d3) noexcept :
    data{d0, d1, d2, d3} {}
  constexpr Vec4f& operator=(const Vec4f &V) noexcept {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
    data[3] = V.data[3];
    return *this; }

  // SIMPLE ACCESSORS & MODIFIERS
  double operator[](int i) const { 
    assert (i >= 0 && i < 4); 
    return data[i]; }
  [[nodiscard]] constexpr double x() const noexcept { return data[0]; }
  [[nodiscard]] constexpr double y() const noexcept { return data[1]; }
  [[nodiscard]] constexpr double z() const noexcept { return data[2]; }
  [[nodiscard]] constexpr double w() const noexcept { return data[3]; }
  [[nodiscard]] constexpr double r() const noexcept { return data[0]; }
  [[nodiscard]] constexpr double g() const noexcept { return data[1]; }
  [[nodiscard]] constexpr double b() const noexcept { return data[2]; }
  [[nodiscard]] constexpr double a() const noexcept { return data[3]; }
  constexpr void setx(double x) noexcept { data[0]=x; }
  constexpr void sety(double y) noexcept { data[1]=y; }
  constexpr void setz(double z) noexcept { data[2]=z; }
  constexpr void setw(double w) noexcept { data[3]=w; }
  constexpr void set(double d0, double d1, double d2, double d3) noexcept {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2;
    data[3] = d3; }

  // ----------------
  // EQUALITY TESTING
  constexpr bool operator==(const Vec4f &V) const noexcept {
    return data[0] == V.data[0] &&
	    data[1] == V.data[1] &&
	    data[2] == V.data[2] &&
	    data[3] == V.data[3]; }
  constexpr bool operator!=(const Vec4f &V) const noexcept {
    return data[0] != V.data[0] ||
	    data[1] != V.data[1] ||
	    data[2] != V.data[2] ||
	    data[3] != V.data[3]; }

  // ------------------------
  // COMMON VECTOR OPERATIONS
  [[nodiscard]] constexpr double Length() const noexcept {
    return sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]+data[3]*data[3]); }
  constexpr void Normalize() noexcept {
    double l = Length();
    if (l > 0) {
      data[0] /= l;
      data[1] /= l;
      data[2] /= l; }}
  constexpr void Scale(double d) noexcept { Scale(d,d,d,d); }
  constexpr void Scale(double d0, double d1, double d2, double d3) noexcept {
    data[0] *= d0;
    data[1] *= d1;
    data[2] *= d2;
    data[3] *= d3; }
  constexpr void Negate() noexcept { Scale(-1.0); }
  [[nodiscard]] constexpr double Dot4(const Vec4f &V) const noexcept {
    return data[0] * V.data[0] +
      data[1] * V.data[1] +
      data[2] * V.data[2] +
      data[3] * V.data[3]; }
  static void Cross3(Vec4f &c, const Vec4f &v1, const Vec4f &v2) noexcept {
    double x = v1.data[1]*v2.data[2] - v1.data[2]*v2.data[1];
    double y = v1.data[2]*v2.data[0] - v1.data[0]*v2.data[2];
    double z = v1.data[0]*v2.data[1] - v1.data[1]*v2.data[0];
    c.data[0] = x; c.data[1] = y; c.data[2] = z; c.data[3] = 1; }
  constexpr void DivideByW() noexcept {
    if (data[3] != 0) {
      data[0] /= data[3];
      data[1] /= data[3];
      data[2] /= data[3];
    } else {
      data[0] = data[1] = data[2] = 0; }
    data[3] = 1; }

  // ---------------------
  // VECTOR MATH OPERATORS
  constexpr Vec4f& operator+=(const Vec4f &V) noexcept {
    data[0] += V.data[0];
    data[1] += V.data[1];
    data[2] += V.data[2];
    data[3] += V.data[3];
    return *this; }
  constexpr Vec4f& operator-=(const Vec4f &V) noexcept {
    data[0] -= V.data[0];
    data[1] -= V.data[1];
    data[2] -= V.data[2];
    data[3] -= V.data[3];
    return *this; }
  constexpr Vec4f& operator*=(double d) noexcept {
    data[0] *= d;
    data[1] *= d;
    data[2] *= d;
    data[3] *= d;
    return *this; }
  constexpr Vec4f& operator/=(double d) noexcept {
    data[0] /= d;
    data[1] /= d;
    data[2] /= d;
    data[3] /= d;
    return *this; }

  // --------------
  // INPUT / OUTPUT
  friend std::ostream& operator<<(std::ostream &ostr, const Vec4f &v) {
    ostr << v.data[0] << " " << v.data[1] << " " << v.data[2] << " " << v.data[3] << std::endl; 
    return ostr; }
  friend std::istream& operator>>(std::istream &istr, Vec4f &v) {
    istr >> v.data[0] >> v.data[1] >> v.data[2] >> v.data[3];
    return istr; }

private:

  friend class Matrix;

  // REPRESENTATION
  double data[4];

};

// ====================================================================
// ====================================================================

#endif
