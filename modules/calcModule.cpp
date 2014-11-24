#include "modules.h"
#include "bipartite.h"

static PyObject* py_compute_bipartite(PyObject* self, PyObject* args)
{
    char const* filegraph;
    char const* path_directory;
    unsigned int type, limit;
    int random;
    PyArg_ParseTuple(args, "ssiii", &path_directory, &filegraph, &type, &random, &limit);

    bool r = false;
    if(random)
        r = true;

    Repository R(path_directory);
    Bipartite B(filegraph, type, r);
    B.run(limit);
    B.save_metrics(&R);
    B.save_nodes(&R);
    B.save_projections(&R);
    B.save_global_metrics(&R);    
}

/* ** ** ** *** * ACCESSORS * ** ** ** *** * */

// INITIALISATION DEFINITION PYTHON
static PyMethodDef calcModule_methods[] = {
    {"compute_bipartite", py_compute_bipartite, METH_VARARGS},
	{NULL, NULL}
};

PyMODINIT_FUNC initcalcModule()
{
	(void) Py_InitModule("calcModule", calcModule_methods);
}

