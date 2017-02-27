		     Using MPI FFTW from Fortran

FFTW 2.1.3 contains *experimental* wrapper functions for calling the
MPI FFTW routines from Fortran.  We are interested in feedback on the
wrapper interface, as well as on whether or not these routines work
for you.  The interface may change in future releases.

You should first read the "Calling FFTW from Fortran" section of the
FFTW manual, as most of what is said there also applies here.  You
should also read the "MPI FFTW" section of the manual.

Wrapper routines:

The wrapper routines have the same names as the corresponding FFTW
routines, but prefixed with "fftw_f77" or "rfftw_f77"
(e.g. fftw_f77_mpi_create_plan).  The parameters are the same, with
the following exceptions:

1) The caveats described in "Calling FFTW from Fortran" hold here as
well; e.g. function return values become the first parameter.

2) The MPI transform routines take a "work" parameter, which can be
NULL in C.  Since there is no way to pass NULL from Fortran, the work
parameter is followed by an *extra* parameter, use_work, which should
be either 0 or 1.  If use_work is 0, then the work parameter is
ignored (just like passing NULL in C), and if use_work is 1, the work
parameter should be an array of the same size as the local data (just
like when work is non-NULL in C).

3) Use the following definitions:

      integer FFTW_TRANSPOSED_ORDER, FFTW_NORMAL_ORDER
      parameter(FFTW_TRANSPOSED_ORDER=1, FFTW_NORMAL_ORDER=0)
      
      integer FFTW_SCRAMBLED_INPUT, FFTW_SCRAMBLED_OUTPUT
      parameter(FFTW_SCRAMBLED_INPUT=8192)
      parameter(FFTW_SCRAMBLED_OUTPUT=16384)

4) The array is divided among the processors for fftwnd in the last
dimension, not the first.  See below.

MPI Communicator Parameters:

It's not clear how one passes MPI_Comm parameters from Fortran to C.
Your feedback on how this can be done in various MPI implementations
would be appreciated--especially ways for the configure script to
automatically detect how it is being done.

MPICH, for example, has an MPI_Comm_f2c macro that is used to convert
between the two representations--the configure script detects this and
uses it if available.

Otherwise, we currently ignore the comm parameter and use a default of
MPI_COMM_WORLD.

Allocating Arrays:

Just as in C, you have to call FFTW at runtime to find out the portion
of the array local to each process (using fftw_f77_mpi_local_sizes,
etcetera).  This means, however, that you have to allocate your data
(and work) arrays dynamically.  There are a few options for doing this:

1) Use Fortran 90, which allows dynamically-allocated arrays.

2) Use a "compile-twice" scheme: Run the program once to get & output
the local size by calling the *_mpi_local_sizes routine.  Then,
substitute this value into the array declaration and recompile.  Since
the local size may be different on different processors, and you only
compile one executable, you will need to take the maximum of the local
sizes when declaring the array.

3) Use C.

The arguments to fftwnd_f77_mpi_local_sizes are interpreted as follows:

call fftwnd_f77_mpi_local_sizes(plan,
                                local_nlast,
                                local_last_start,
                                local_nlast2_after_transform,
                                local_last2_start_after_transform,
                                total_local_size)

As in C, your array must contain at least total_local_size elements
(which may be slightly larger than you expect).  "last" refers to the
last dimension of your data and "last2" refers to the second-to-last
dimension (for when you do the transform with transposed-order
output)--this it the opposite of C, where those arguments refer to the
first and second dimensions, respectively.

For example, if you are doing a 3d nx x ny x nz transform, your array
will be declared as data(total_local_size) and should be interpreted
as a Fortran (column-major) array data(0:nx-1, 0:ny-1,
0:local_nlast-1), where the last dimension corresponds to the nz
indices starting at local_last_start.  Note that the indices are
zero-based.  If you transform the array with transposed-order output,
the array should then be interpreted as data(0:nx-1, 0:nz-1,
0:local_nlast2_after_transform-1), where the last dimension
corresponds to the ny indices starting at
local_last2_start_after_transform.
