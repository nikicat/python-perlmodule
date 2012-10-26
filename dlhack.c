#include <Python.h>
#include <dlfcn.h>
#include "2to3.h"

/* This is a fake perl module that will look for the real thing ('perl2.so')
 * in sys.path and then load this one with the RTLD_GLOBAL set in order to
 * make the symbols available for extension modules that perl might load.
 */


#if PY_MAJOR_VERSION >= 3
#define INITERROR return NULL;
extern PyObject* PyInit_perl()
#else
#define INITERROR return;
extern void initperl()
#endif
{
    void* handle;
    int i, npath;
    char buf[1024];

    PyObject *path = PySys_GetObject("path");
    if (path == NULL || !PyList_Check(path)) {
        PyErr_SetString(PyExc_ImportError, "sys.path must be a list of directory names");
        INITERROR;
    }

    PyObject* m = PyImport_AddModule("__main__");
    if (m == NULL) {
        PyErr_SetString(PyExc_ImportError, "failed to import __main__ module");
        INITERROR;
    }
    PyObject* d = PyModule_GetDict(m);
    PyObject* res = PyRun_String("__import__('sysconfig').get_config_var('SO')", Py_eval_input, d, d);
    if (res == NULL) {
        PyErr_SetString(PyExc_ImportError, "could not retrieve SO system config variable");
        INITERROR;
    }
    char* soname = PyString_AsUTF8(res);
    if (soname == NULL) {
        PyErr_SetString(PyExc_ImportError, "could not convert SO system config variable to string");
        INITERROR;
    }
    npath = PyList_Size(path);
    for (i = 0; i < npath; i++) {
        PyObject *v = PyList_GetItem(path, i);
        if (!PyString_Check(v))
            continue;
        Py_ssize_t size;
        strcpy(buf, PyString_AsUTF8AndSize(v, &size));
        if (size + strlen(soname) + 6 >= sizeof(buf))
            continue; /* Too long */
        if (buf[0] != '/')
            continue; /* Not absolute */
        if (strlen(buf) != size)
            continue; /* v contains '\0' */
        strcpy(buf+size, "/perl2");
        strcpy(buf+size+6, soname);

        handle = dlopen(buf, RTLD_NOW | RTLD_GLOBAL);
        if (handle) {
            PyObject* (*f)() = dlsym(handle, "initperl2");
            if (f) {
                return f();
            }
            else {
                PyErr_SetString(PyExc_ImportError, "initperl2 entry point not found");
                INITERROR;
            }
        }
    }
    char* err = dlerror();
    PyErr_SetString(PyExc_ImportError, err ? err : "general failure");
    INITERROR;
}
