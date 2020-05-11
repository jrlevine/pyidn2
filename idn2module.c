/*
 * Wrapper around GNU libidn2, John Levine, May 2020
 *
 * Provides four functions, utoa(), lookup(), register(), atou()
 * and exception idn2.error raised when the libidn2 routines return an
 * error code
  
 * Copyright 2019-2020 Standcore LLC

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.

 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include <Python.h>

#include <idn2.h>

static PyObject *Idn2Error;

/* turn idn2 error into an error message
 */
void
   setidn2errstr(int rc)
{
	char *errstr = NULL;

	switch(rc) {
		case IDN2_MALLOC:
			errstr = "Memory allocation error."; break;
		case IDN2_NO_CODESET:
			errstr = "Could not determine locale string encoding format."; break;
		case IDN2_ICONV_FAIL:
			errstr = "Could not transcode locale string to UTF-8."; break;
		case IDN2_ENCODING_ERROR:
			errstr = "Unicode data encoding error."; break;
		case IDN2_NFC:
			errstr = "Error normalizing string."; break;
		case IDN2_PUNYCODE_BAD_INPUT:
			errstr = "Punycode invalid input."; break;
		case IDN2_PUNYCODE_BIG_OUTPUT:
			errstr = "Punycode output buffer too small."; break;
		case IDN2_PUNYCODE_OVERFLOW:
			errstr = "Punycode conversion would overflow."; break;
		case IDN2_TOO_BIG_DOMAIN:
			errstr = "Domain name longer than 255 characters."; break;
		case IDN2_TOO_BIG_LABEL:
			errstr = "Domain label longer than 63 characters."; break;
		case IDN2_INVALID_ALABEL:
			errstr = "Input A-label is not valid."; break;
		case IDN2_UALABEL_MISMATCH:
			errstr = "Input A-label and U-label does not match."; break;
		case IDN2_INVALID_FLAGS:
			errstr = "Invalid combination of flags."; break;
		case IDN2_NOT_NFC:
			errstr = "String is not NFC."; break;
		case IDN2_2HYPHEN:
			errstr = "String has forbidden two hyphens."; break;
		case IDN2_HYPHEN_STARTEND:
			errstr = "String has forbidden starting/ending hyphen."; break;
		case IDN2_LEADING_COMBINING:
			errstr = "String has forbidden leading combining character."; break;
		case IDN2_DISALLOWED:
			errstr = "String has disallowed character."; break;
		case IDN2_CONTEXTJ:
			errstr = "String has forbidden context-j character."; break;
		case IDN2_CONTEXTJ_NO_RULE:
			errstr = "String has context-j character with no rull."; break;
		case IDN2_CONTEXTO:
			errstr = "String has forbidden context-o character."; break;
		case IDN2_CONTEXTO_NO_RULE:
			errstr = "String has context-o character with no rull."; break;
		case IDN2_UNASSIGNED:
			errstr = "String has forbidden unassigned character."; break;
		case IDN2_BIDI:
			errstr = "String has forbidden bi-directional properties."; break;
		case IDN2_DOT_IN_LABEL:
			errstr = "Label has forbidden dot."; break;
		case IDN2_INVALID_TRANSITIONAL:
			errstr = "Label has character forbidden in transitional mode."; break;
		case IDN2_INVALID_NONTRANSITIONAL:
			errstr = "Label has character forbidden in non-transitional mode)."; break;
		default:
			PyErr_SetObject(Idn2Error, PyUnicode_FromFormat("idn2 error %d", rc));
			return;
	}
	/* use known error string */
	PyErr_SetString(Idn2Error, errstr);
}

/*
 * utoa(str ulabel) -> bytesa-label
 */
static PyObject *
   idn2_utoa(PyObject *self, PyObject *args)
{
	const char *ulabel;
	char *alabel = NULL;
	PyObject *ret;
	int rc;

	if (!PyArg_ParseTuple(args, "s", &ulabel))
		return NULL;
	rc = idn2_to_ascii_8z(ulabel, &alabel, IDN2_NO_TR46);
	
	if(rc == IDN2_OK) {
		ret = Py_BuildValue("y", alabel);
		free(alabel);
		return ret;
	}
	setidn2errstr(rc);
	return NULL;
}

/*
 * lookup(str ulabel) -> bytes alabel
 */
static PyObject *
   idn2_lookup(PyObject *self, PyObject *args)
{
	const char *ulabel;
	char *alabel = NULL;
	PyObject *ret;
	int rc;

	if (!PyArg_ParseTuple(args, "s", &ulabel))
		return NULL;
	rc = idn2_lookup_u8((const uint8_t *)ulabel, (uint8_t **)&alabel, IDN2_NFC_INPUT);

	if(rc == IDN2_OK) {
		ret = Py_BuildValue("y", alabel);
		free(alabel);
		return ret;
	}
	setidn2errstr(rc);
	return NULL;
}

/*
 * register(str ulabel, bytes alabel) -> bytes rlabel
 */
static PyObject *
   idn2_register(PyObject *self, PyObject *args)
{
	const char *ulabel, *alabel;
	int *alen;
	char *rlabel = NULL;
	PyObject *ret;
	int rc;

	if (!PyArg_ParseTuple(args, "zz#", &ulabel, &alabel, &alen))
		return NULL;
	if(!ulabel && !alabel) {
		PyErr_SetString(Idn2Error, "Both arguments null.");
		return NULL;
	}

	rc = idn2_register_u8((const uint8_t *)ulabel, (const uint8_t *)alabel, (uint8_t **)&rlabel, IDN2_NFC_INPUT);

	if(rc == IDN2_OK) {
		ret = Py_BuildValue("y", rlabel);
		free(rlabel);
		return ret;
	}
	setidn2errstr(rc);
	return NULL;
}

/*
 * atou(bytes alabel) -> str ulabel
 */
static PyObject *
   idn2_atou(PyObject *self, PyObject *args)
{
	const char *alabel;
	char *ulabel = NULL;
	PyObject *ret;
	int rc;

	if (!PyArg_ParseTuple(args, "y", &alabel))
		return NULL;
	rc = idn2_to_unicode_8z8z(alabel, &ulabel, 0);

	if(rc == IDN2_OK) {
		ret = PyUnicode_FromString(ulabel);
		free(ulabel);
		return ret;
	}
	setidn2errstr(rc);
	return NULL;
}

static PyMethodDef Idn2Methods[] = {
	{"utoa",  idn2_utoa, METH_VARARGS, "Convert U-labels to A-labels."},
	{"lookup",  idn2_lookup, METH_VARARGS, "Lookup U-label conversion."},
	{"register",  idn2_register, METH_VARARGS, "Lookup U-label conversion."},
	{"atou",  idn2_atou, METH_VARARGS, "Convert A-labels to U-labels."},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef idn2module = {
	PyModuleDef_HEAD_INIT,
	"idn2",   /* name of module */
	NULL, /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
	or -1 if the module keeps state in global variables. */
	Idn2Methods
};

PyMODINIT_FUNC
   PyInit_idn2(void)
{
	PyObject *m;

	m = PyModule_Create(&idn2module);
	if (m == NULL)
		return NULL;

	Idn2Error = PyErr_NewException("idn2.error", NULL, NULL);
	if(Idn2Error == NULL) return NULL;
	Py_INCREF(Idn2Error);
	if(PyModule_AddObject(m, "error", Idn2Error) < 0) return NULL;

	/* stuff */
	return m;
}
