#include <Python.h>
#include <dlfcn.h>

/* This is a fake perl module that will look for the real thing ('perl2.so')
 * in sys.path and then load this one with the RTLD_GLOBAL set in order to
 * make the symbols available for extension modules that perl might load.
 */

extern PyObject* PyInit_perl()
{
    void* handle;
    int i, npath;
    char buf[1024];

    PyObject *path = PySys_GetObject("path");
    if (path == NULL || !PyList_Check(path)) {
        PyErr_SetString(PyExc_ImportError,
                "sys.path must be a list of directory names");
        return NULL;
    }

    npath = PyList_Size(path);
    for (i = 0; i < npath; i++) {
        PyObject *v = PyList_GetItem(path, i);
        if (!PyUnicode_Check(v))
            continue;
        Py_ssize_t size;
        strcpy(buf, PyUnicode_AsUTF8AndSize(v, &size));
        if (size + 10 >= sizeof(buf))
            continue; /* Too long */
        if (buf[0] != '/')
            continue; /* Not absolute */
        if (strlen(buf) != size)
            continue; /* v contains '\0' */
        strcpy(buf+size, "/perl2.cpython-33m.so");

        handle = dlopen(buf, RTLD_NOW | RTLD_GLOBAL);
        if (handle) {
            PyObject* (*f)() = dlsym(handle, "initperl2");
            if (f) {
                return f();
            }
            else {
                PyErr_SetString(PyExc_ImportError, "initperl2 entry point not found");
                return NULL;
            }
        }
    }
    char* err = dlerror();
    PyErr_SetString(PyExc_ImportError, err ? err : "general failure");
    return NULL;
}
