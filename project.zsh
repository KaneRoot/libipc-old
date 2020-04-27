
package=libipc    # Package name.
version=0.5.1     # Package version.

# Our targets are the library and its documentation.
targets=(libipc man/libipc.7)

# The libipc target is a library ("for the C language" is implied).
# The `library` type automatically adds tho targets:
#   `target`.so  of type `sharedlib`
#   `target`.a   of type `staticlib`
type[libipc]=library
# Sources are added by default to the tarball.
sources[libipc]="$(ls src/*.c)"

depends[libipc]=$(ls src/*.h)

# We need to add extra compilation flags.
variables+=(CFLAGS "-Wall -Wextra -g")

# Let's add some CFLAGS, with another syntax.
cflags[libipc]="-std=c11"

# The man/libipc.7 target is a manual generated with `scdoc`.
# `scdocman` is one of the many back-ends of build.zsh: https://git.baguette.netlib.re/Baguette/build.zsh
type[man/libipc.7]=scdocman

# Finally, we tell what we want in the tarball, generated with `make dist`.
dist=(libipc.so libipc.a)     # The library in both shared and static versions.
dist+=(man/*.scd man/*.[1-9]) # Manual pages (and sources).
dist+=(Makefile project.zsh)  # The generated Makefile and this project.zsh.
