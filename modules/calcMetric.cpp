#include <Python.h>
#include "modules.h"

static PyObject* py_arrange_metrics(PyObject* self, PyObject* args)
{
	PyObject* dict_metric_1, * dict_metric_2;
    PyArg_ParseTuple(args, "OO", &dict_metric_1, &dict_metric_2);

    PyObject* new_dict_metric_1 = PyDict_New();
    PyObject* new_dict_metric_2 = PyDict_New();
    dict_metric_2 = PyDict_Copy(dict_metric_2);

    PyObject* key, * value;
	Py_ssize_t pos = 0;
    while(PyDict_Next(dict_metric_1, &pos, &key, &value) && PyDict_Size(dict_metric_2) > 0)
    {
    	if(PyDict_Contains(dict_metric_2, key))
    	{
    		PyDict_SetItem(new_dict_metric_1, key, value);
    		PyDict_SetItem(new_dict_metric_2, key, PyDict_GetItem(dict_metric_2, key));
    		PyDict_DelItem(dict_metric_2, key);
    	}
    }

    PyObject* lst = PyList_New(2);
    PyList_SET_ITEM(lst, 0, new_dict_metric_1);
    PyList_SET_ITEM(lst, 1, new_dict_metric_2);

    return lst;
}

// INITIALISATION DEFINITION PYTHON
static PyMethodDef calcMetric_methods[] = {
    {"arrange_metrics", py_arrange_metrics, METH_VARARGS},   
	{NULL, NULL}
};

PyMODINIT_FUNC initcalcMetric()
{
	(void) Py_InitModule("calcMetric", calcMetric_methods);
}
