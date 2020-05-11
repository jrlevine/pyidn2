# pyidn2
python wrapper for GNU libidn2

Exports four routines and error class idn2.error

## Routines

`utoa(str ulabel) -> bytes alabel`

`lookup(str ulabel) -> bytes alabel`

`register(str ulabel, bytes alabel) -> bytes rlabel`

`atou(bytes alabel) -> str ulabel`

In register, either argument can be None.  If both are present they have to agree.

## License

Two-clause BSD
