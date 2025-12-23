%module(docstring="This is a Python wrapper for SPS signals") swig_signals
#pragma SWIG nowarn=320
%{

  #define SWIG_FILE_WITH_INIT
  // Try to ignore cast between pointer-to-function and pointer-to-object
  #include <sps/msignals.hpp>
%}

// SWIG 4.1+ compatibility - SWIG_Python_AppendOutput now requires 3 args
%insert("header") %{
#if SWIG_VERSION >= 0x040100
  #define SWIG_AppendOutput(result, obj) SWIG_Python_AppendOutput(result, obj, 0)
#else
  #define SWIG_AppendOutput(result, obj) SWIG_Python_AppendOutput(result, obj)
#endif
%}

#ifdef _SWIG_WIN32
%include "windows.i"
#endif

%include "carrays.i"
%array_class(float, floatArrayClass);
%array_class(double, doubleArrayClass);
%array_class(size_t, sizetArrayClass);
%array_functions(float, floatArray);
%array_functions(double, doubleArray);

%include "std_complex.i"

%include "numpy.i"
%init {
  import_array();
}

#define STATIC_INLINE_BEGIN
#define STATIC_INLINE_END

// Not defined unless sps/export.h is included
#define SPS_EXPORT

// Individual modules
%include <sps/msignals.hpp>

namespace sps {
  %template(DivideArrayFloat) DivideArray<float>;
  %template(DivideArrayDouble) DivideArray<double>;

  //%template(Signal1DPlanFloat) Signal1DPlan<float>;
  //%template(Signal1DPlanDouble) Signal1DPlan<double>;

  %template(Signal1DFloat) signal1D<float>;
  %template(Signal1DComplexFloat) signal1D<std::complex<float> >;

  %template(ConvFloat) conv<float>;

  %template(ConvFFTFloat) conv_fft<float>;
  %template(FFTFloat) fft<float>;
  %template(IFFTFloat) ifft<float>;

  %template(UnpackFloat) unpack_c2r<float>;
  %template(PackFloat)   pack_r2c<float>;

  %template(MSignal1DFloat) msignal1D<float>;
  // Instantiate two overloads
  %template(MConvFFTFloat)  mconv_fft<float>;

  %template(Signal1DDouble) signal1D<double>;
  %template(Signal1DComplexDouble) signal1D<std::complex<double> >;

  %template(ConvDouble) conv<double>;

  %template(ConvFFTDouble) conv_fft<double>;
  %template(FFTDouble) fft<double>;
  %template(IFFTDouble) ifft<double>;

  %template(UnpackDouble) unpack_c2r<double>;
  %template(PackDouble)   pack_r2c<double>;

  %template(MSignal1DDouble) msignal1D<double>;
  // Instantiate two overloads
  %template(MConvFFTDouble)  mconv_fft<double>;   
}
