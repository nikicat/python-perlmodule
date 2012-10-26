static char* PyString_AsUTF8AndSize(PyObject *string, Py_ssize_t* size) {
#if PY_MAJOR_VERSION < 3
    char* res;
    PyString_AsStringAndSize(string, &res, size);
    return res;
#elif PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION < 3
    PyObject* bytes = PyUnicode_AsUTF8String(string);
    *size = PyBytes_Size(bytes);
    return PyBytes_AsString(bytes);
#else
    return PyUnicode_AsUTF8AndSize(string, size);
#endif
}

static char* PyString_AsUTF8(PyObject *string) {
#if PY_MAJOR_VERSION < 3
    return PyString_AsString(string);
#elif PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION < 3
    return PyBytes_AsString(PyUnicode_AsUTF8String(string));
#else
    return PyUnicode_AsUTF8(string);
#endif
}

#if PY_MAJOR_VERSION >= 3
#define PyString_Check PyUnicode_Check
#define PyString_FromString PyUnicode_FromString
#define PyString_FromStringAndSize PyUnicode_FromStringAndSize
#endif
