//@generated
#include "emxutil2.hh"
/* Function Definitions */
emxArray_char_T *argInit_1xUnbounded_char_T(int sz)
{
  emxArray_char_T *result;
  int idx0;
  int idx1;

  /* Set the size of the array.
     Change this size to the value that the application requires. */
  result = emxCreate_char_T(1, sz);

  /* Loop over the array to initialize each element. */
  for (idx0 = 0; idx0 < 1; idx0++) {
    for (idx1 = 0; idx1 < result->size[1U]; idx1++) {
      /* Set the value of the array element.
         Change this value to the value that the application requires. */
      result->data[idx1] = argInit_char_T();
    }
  }

  return result;
}

emxArray_real_T *argInit_Unboundedx1_real_T(int sz)
{
  const int iv[1] = { sz };

  emxArray_real_T *result;
  int idx0;

  /* Set the size of the array.
     Change this size to the value that the application requires. */
  result = emxCreateND_real_T(1, iv);

  /* Loop over the array to initialize each element. */
  for (idx0 = 0; idx0 < result->size[0U]; idx0++) {
    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result->data[idx0] = argInit_real_T();
  }

  return result;
}

char argInit_char_T(void)
{
  return '\0';
}

double argInit_real_T(void)
{
  return 0.0;
}

signal_dimensions_t argInit_signal_dimensions_t(void)
{
  signal_dimensions_t result;
  unsigned int result_tmp;

  /* Set the value of each structure field.
     Change this value to the value that the application requires. */
  result_tmp = argInit_uint32_T();
  result.fragsize = result_tmp;
  result.wndlen = result_tmp;
  result.fftlen = result_tmp;
  result.channels = result_tmp;
  result.domain = argInit_char_T();
  result.srate = argInit_real_T();
  return result;
}

unsigned int argInit_uint32_T(void)
{
  return 0U;
}

user_config_t argInit_user_config_t(void)
{
  user_config_t result;

  /* Set the value of each structure field.
     Change this value to the value that the application requires. */
  result.name = argInit_1xUnbounded_char_T();
  result.value = c_argInit_UnboundedxUnbounded_r();
  return result;
}

emxArray_user_config_t *c_argInit_Unboundedx1_user_conf(int sz)
{
  const int iv[1] = { sz };

  emxArray_user_config_t *result;
  int idx0;

  /* Set the size of the array.
     Change this size to the value that the application requires. */
  result = emxCreateND_user_config_t(1, iv);

  /* Loop over the array to initialize each element. */
  for (idx0 = 0; idx0 < result->size[0U]; idx0++) {
    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result->data[idx0] = argInit_user_config_t();
  }

  return result;
}

emxArray_real_T *c_argInit_UnboundedxUnbounded_r(int sx, int sy)
{
  emxArray_real_T *result;
  int idx0;
  int idx1;

  /* Set the size of the array.
     Change this size to the value that the application requires. */
  result = emxCreate_real_T(sx, sy);

  /* Loop over the array to initialize each element. */
  for (idx0 = 0; idx0 < result->size[0U]; idx0++) {
    for (idx1 = 0; idx1 < result->size[1U]; idx1++) {
      /* Set the value of the array element.
         Change this value to the value that the application requires. */
      result->data[idx0 + result->size[0] * idx1] = argInit_real_T();
    }
  }

  return result;
}
