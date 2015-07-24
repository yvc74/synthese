/** PythonInterpreter class implementation.
	@file PythonInterpreter.cpp

	This file belongs to the SYNTHESE project (public transportation specialized software)
	Copyright (C) 2002 Hugues Romain - RCSmobility <contact@rcsmobility.com>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "PythonInterpreter.hpp"
#include "Factory.h"
#include "Function.h"
#include "Action.h"
#include "Request.h"

#include <structmember.h>
#include <iostream>


namespace synthese
{
	using namespace util;
	using namespace server;

	namespace cms
	{

		const std::string PythonInterpreter::INTERPRETER_NAME = "python";

		const std::string PythonInterpreter::MODULE_NAME = "synthese";

		// Declaration of bindings to SYNTHESE APIs
		PyMethodDef PythonInterpreter::PYTHON_BINDINGS[] = {
			{
				"service",
				synthese::cms::PythonInterpreter::BindingService,
				METH_VARARGS,
				"Invoke a SYNTHESE service"
			},
			{
				"action",
				PythonInterpreter::BindingAction,
				METH_VARARGS,
				"Invoke a SYNTHESE action"
			},
			/*
			{
				"include",
				PythonInterpreter::BindingInclude,
				METH_VARARGS,
				"Include the content of a SYNTHESE page into the script"
			},
			*/
			{NULL, NULL, 0, NULL}
		};


		// OutputStreamWrapper is a Python extension class that wraps a C++ ostream into an interface compatible with Python stdout and stderr
		typedef struct {
			PyObject_HEAD
			std::ostream* output;
		} OutputStreamWrapper;


		static void
		OutputStreamWrapper_dealloc(OutputStreamWrapper* self)
		{
			self->ob_type->tp_free((PyObject*)self);
		}


		static PyObject *
		OutputStreamWrapper_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
		{
			OutputStreamWrapper *self;
			self = (OutputStreamWrapper *)type->tp_alloc(type, 0);
			return (PyObject *)self;
		}


		static int
		OutputStreamWrapper_init(OutputStreamWrapper *self, PyObject *args, PyObject *kwds)
		{
			return 0;
		}


		static PyObject *
		OutputStreamWrapper_write(OutputStreamWrapper* self, PyObject* args)
		{
			char* toWrite = NULL;

			if(!PyArg_ParseTuple(args, "s", &toWrite))
			{
				// TODO : error management
				return NULL;
			}

			// Write argument to output
			if(NULL != self->output)
			{
				*(self->output) << toWrite;
			}

			Py_DECREF(args);

			Py_RETURN_NONE;
		}


		static PyMethodDef OutputStreamWrapper_methods[] = {
			{
				"write",
				(PyCFunction)OutputStreamWrapper_write,
				METH_VARARGS,
				"Write a string to the wrapped C++ ostream"
			},
			{NULL, NULL, 0, NULL}
		};


		static PyTypeObject OutputStreamWrapperType = {
			PyObject_HEAD_INIT(NULL)
			0,                          /*ob_size*/
			"synthese.OutputStreamWrapper", /*tp_name*/
			sizeof(OutputStreamWrapper),/*tp_basicsize*/
			0,                          /*tp_itemsize*/
			(destructor)OutputStreamWrapper_dealloc, /*tp_dealloc*/
			0,                          /*tp_print*/
			0,                          /*tp_getattr*/
			0,                          /*tp_setattr*/
			0,                          /*tp_compare*/
			0,                          /*tp_repr*/
			0,                          /*tp_as_number*/
			0,                          /*tp_as_sequence*/
			0,                          /*tp_as_mapping*/
			0,                          /*tp_hash */
			0,                          /*tp_call*/
			0,                          /*tp_str*/
			0,                          /*tp_getattro*/
			0,                          /*tp_setattro*/
			0,                          /*tp_as_buffer*/
			Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
			"OutputStreamWrapper",      /* tp_doc */
			0,                          /* tp_traverse */
			0,                          /* tp_clear */
			0,                          /* tp_richcompare */
			0,                          /* tp_weaklistoffset */
			0,                          /* tp_iter */
			0,                          /* tp_iternext */
			OutputStreamWrapper_methods,/* tp_methods */
			0,                          /* tp_members */
			0,                          /* tp_getset */
			0,                          /* tp_base */
			0,                          /* tp_dict */
			0,                          /* tp_descr_get */
			0,                          /* tp_descr_set */
			0,                          /* tp_dictoffset */
			(initproc)OutputStreamWrapper_init,      /* tp_init */
			0,                          /* tp_alloc */
			OutputStreamWrapper_new,    /* tp_new */
			0,                          /* tp_free */
			0,                          /* tp_is_gc */
			0,                          /* tp_bases */
			0,                          /* tp_mro */
			0,                          /* tp_cache */
			0,                          /* tp_subclasses */
			0,                          /* tp_weaklist */
			0,                          /* tp_del */
			0                           /* tp_version_tag */
		};


		void PythonInterpreter::Initialize()
		{
			Py_Initialize();

			// Register a 'synthese' module exposing the bindings to SYNTHESE API
			PyObject* syntheseModule = Py_InitModule(MODULE_NAME.c_str(), PYTHON_BINDINGS);

			if(NULL == syntheseModule)
			{
				Log::GetInstance().error("Failed to initialize PythonInterpreter");
			}

			else
			{	
				// Register the class 'OutputStreamWrapper' as a Python object
				if(PyType_Ready(&OutputStreamWrapperType) == 0)
				{
					Py_INCREF(&OutputStreamWrapperType);
					PyModule_AddObject(syntheseModule, "OutputStreamWrapper", (PyObject*) &OutputStreamWrapperType);
					Log::GetInstance().info("PythonInterpreter successfully initialized");
				}

				else
				{
					Log::GetInstance().error("Failed to initialize PythonInterpreter");
				}
			}
		}


		void PythonInterpreter::End()
		{
			Py_Finalize();
		}


		PyObject* PythonInterpreter::BindingService(PyObject* self, PyObject* args)
		{
			char* serviceName = NULL;
			PyObject* parametersDictionary = NULL;
			Request request;
			boost::shared_ptr<ParametersMap> parametersMap(new ParametersMap);

			if(!PyArg_ParseTuple(args, "sO:service", &serviceName, &parametersDictionary))
			{
				std::string errorMsg = "Call to 'service' from Python script has wrong arguments";
				Log::GetInstance().warn(errorMsg);
				PyErr_SetString(PyExc_RuntimeError, errorMsg.c_str());
				return NULL;
			}

			if(!Factory<Function>::contains(serviceName))
			{
				std::ostringstream warningStream;
				warningStream << "Call to 'service' from Python script with unknown service '" << serviceName << "'";
				Log::GetInstance().warn(warningStream.str());
				PyErr_SetString(PyExc_RuntimeError, warningStream.str().c_str());
				return NULL;
			}

			boost::shared_ptr<Function> function = boost::shared_ptr<Function>(Factory<Function>::create(serviceName));

			// Convert the argument dictionary into a ParametersMap
			if(NULL != parametersDictionary)
			{
				parametersMap = BuildParametersMapFromDictionary(parametersDictionary);
				//Py_DECREF(parametersDictionary);
			}

			parametersMap->insert(Request::PARAMETER_SERVICE, serviceName);
			function->_setFromParametersMap(*parametersMap);

			// TODO : run function with ostringstream output and push output string into result map/dict
			ParametersMap resultMap = function->run(std::cout, request);

			// Convert the result map into a Python dictionary and return it
			PyObject* resultDict = BuildDictionaryFromParametersMap(resultMap);
			return resultDict;
		}


		PyObject* PythonInterpreter::BindingAction(PyObject* self, PyObject* args)
		{
			char* actionName = NULL;
			PyObject* parametersDictionary = NULL;
			Request request;
			boost::shared_ptr<ParametersMap> parametersMap(new ParametersMap);

			if(!PyArg_ParseTuple(args, "sO:action", &actionName, &parametersDictionary))
			{
				std::string errorMsg = "Call to 'action' from Python script has wrong arguments";
				Log::GetInstance().warn(errorMsg);
				PyErr_SetString(PyExc_RuntimeError, errorMsg.c_str());
				return NULL;
			}

			if(!Factory<Action>::contains(actionName))
			{
				std::ostringstream warningStream;
				warningStream << "Call to 'action' from Python script with unknown action '" << actionName << "'";
				Log::GetInstance().warn(warningStream.str());
				PyErr_SetString(PyExc_RuntimeError, warningStream.str().c_str());
				return NULL;
			}

			boost::shared_ptr<Action> action = boost::shared_ptr<Action>(Factory<Action>::create(actionName));

			// Convert the argument dictionary into a ParametersMap
			if(NULL != parametersDictionary)
			{
				parametersMap = BuildParametersMapFromDictionary(parametersDictionary);
				//Py_DECREF(parametersDictionary);
			}

			parametersMap->insert(Request::PARAMETER_ACTION, actionName);
			action->_setFromParametersMap(*parametersMap);
			action->run(request);

			// Actions do not return parameters maps, but they may store attributes in the parameters map of the request
			PyObject* resultDict = BuildDictionaryFromParametersMap(request.getParametersMap());
			return resultDict;
		}


		PyObject* PythonInterpreter::BindingInclude(PyObject* self, PyObject* args)
		{
			// TODO :
			// * extract parameters 'smartURL'=string and 'parametersMap'=dictionary
			// * build full smart URL by prepending '!'
			// * retrieve Webpage using smart URL => /!\ requires reference to current page
			// * invoke display() on the target page =>
			//     - /!\ no stream at this stage of the execution (Python scripts output to stdout which is redirected to stream)
			//     - /!\ not sure if 'locals' of calling script are transmitted to included script
			// => cannot implement same semantics as CSM <##>

			return NULL;
		}


		void PythonInterpreter::Execute(
			const std::string& script,
			std::ostream& stream,
			const util::ParametersMap& parameters,
			util::ParametersMap& variables
		)
		{
			// TODO : see what kind of error management is implemented into CMS and try to provide same level

			// 'globals' is the dictionary containing the main functions of Python
			PyObject* mainModule = PyImport_AddModule("__main__");
			PyObject* globals    = PyModule_GetDict(mainModule);

			// 'locals' is the dictionary containing the input parameters of the script
			// It will store the return values of the script
			PyObject* locals  = BuildDictionaryFromParametersMap(parameters);

			// Redirect Python stdout to stream using an OutputStreamWrapper
			PyObject* sys = PyImport_ImportModule("sys");
			PyObject* wrapper = PyObject_CallObject((PyObject*) &OutputStreamWrapperType, NULL);
			((OutputStreamWrapper*) wrapper)->output = &stream;
			if (-1 == PyObject_SetAttrString(sys, "stdout", wrapper)) {
				Log::GetInstance().warn("Failed to redirect Python stdout to stream");
			}

			// Execute the script
			PyRun_String(script.c_str(), Py_file_input, globals, locals);
			PyObject* exception = PyErr_Occurred();

			if(NULL != exception)
			{
				// TODO : log properly into SYNTHESE logs (requires another wrapper for Python stderr that points to SYNTHESE logger)
				PyErr_Print();
			}

			// Write return values into the map 'variables'
			boost::shared_ptr<util::ParametersMap> returnMap = BuildParametersMapFromDictionary(locals);
			variables.clear();
			variables.merge(*returnMap, "", true);

			// Memory management : decrement reference to 'locals'
			Py_DECREF(locals);
			Py_XDECREF(sys);
			Py_XDECREF(wrapper);
		}


		PyObject* PythonInterpreter::BuildDictionaryFromParametersMap(const ParametersMap& map)
		{
			PyObject* dictionary = PyDict_New();
			_BuildDictionaryFromParametersMap(dictionary, map);
			return dictionary;
		}


		void PythonInterpreter::_BuildDictionaryFromParametersMap(PyObject* dictionary, const ParametersMap& map)
		{
			// 1) Insert fields into the dictionary
			ParametersMap::Map fieldMap = map.getMap();
			ParametersMap::Map::const_iterator it;

			for(it = fieldMap.begin(); it != fieldMap.end(); ++it)
			{
				PyObject* key = Py_BuildValue("s", it->first.c_str());
				PyObject* value = Py_BuildValue("s", it->second.c_str());
				PyDict_SetItem(dictionary, key, value);
			}

			// 2) Iterate over submaps
			ParametersMap::SubMapsKeys subMapsKeys = map.getSubMapsKeys();
			ParametersMap::SubMapsKeys::const_iterator itSubMapsKeys;

			// Each submap key can be used multiple times, which cannot be represented as-is in a dictionary
			// To process this case :
			// * a tuple is created for each submap key
			// * each submap is converted as a dictionary using this algorithm and inserted in the matching tuple
			// * the tuple is inserted in the main dictionary using the shared submap key

			for(itSubMapsKeys = subMapsKeys.begin(); itSubMapsKeys != subMapsKeys.end(); ++itSubMapsKeys)
			{
				const ParametersMap::SubParametersMap::mapped_type& subMaps = map.getSubMaps(*itSubMapsKeys);
				ParametersMap::SubParametersMap::mapped_type::const_iterator itSubMaps;
				PyObject* subDictionariesTuple = PyTuple_New(subMaps.size());
				unsigned int tupleIndex = 0;

				for(itSubMaps = subMaps.begin(); itSubMaps != subMaps.end(); ++itSubMaps)
				{
					PyObject* subDictionary = PyDict_New();
					_BuildDictionaryFromParametersMap(subDictionary, **itSubMaps);
					PyTuple_SetItem(subDictionariesTuple, tupleIndex, subDictionary);
					tupleIndex++;
				}

				PyDict_SetItemString(dictionary, itSubMapsKeys->c_str(), subDictionariesTuple);
			}
		}


		boost::shared_ptr<ParametersMap> PythonInterpreter::BuildParametersMapFromDictionary(PyObject* dictionary)
		{
			boost::shared_ptr<ParametersMap> parametersMap(new ParametersMap());
			_BuildParametersMapFromDictionary(parametersMap, dictionary);
			return parametersMap;
		}


		void PythonInterpreter::_BuildParametersMapFromDictionary(boost::shared_ptr<ParametersMap> map, PyObject* dictionary)
		{
			PyObject*  key = NULL;
			PyObject*  value = NULL;
			Py_ssize_t index = 0;

			// These objects are used for type identification only
			PyObject*  tuple     = PyTuple_New(1);
			PyObject*  dict      = PyDict_New();
			PyObject*  tupleType = PyObject_Type(tuple);
			PyObject*  dictType  = PyObject_Type(dict);

			while(PyDict_Next(dictionary, &index, &key, &value))
			{
				if(NULL == value)
				{
					continue;
				}

				if(1 == PyObject_IsInstance(value, tupleType))
				{
					// Value is a tuple : create new submaps named 'key' for each element of the tuple
					Py_ssize_t nbSubmaps = PyTuple_Size(value);

					for(int i = 0; i < nbSubmaps; i++)
					{
						PyObject* keyStrPython = PyObject_Str(key);
						std::string keyString = PyString_AsString(keyStrPython);
						PyObject* submap = PyTuple_GetItem(value, i);

						// Build the new submap from the content of the current tuple index
						boost::shared_ptr<ParametersMap> subParametersMap(new ParametersMap());
						_BuildParametersMapFromDictionary(subParametersMap, submap);
						map->insert(keyString, subParametersMap);
						Py_XDECREF(keyStrPython);
					}
				}

				else if(1 == PyObject_IsInstance(value, dictType))
				{
					// Value is a dictionary : create one submap named 'key'
					PyObject* keyStrPython = PyObject_Str(key);
					std::string keyString = PyString_AsString(keyStrPython);
					boost::shared_ptr<ParametersMap> subParametersMap(new ParametersMap());
					_BuildParametersMapFromDictionary(subParametersMap, value);
					map->insert(keyString, subParametersMap);
					Py_XDECREF(keyStrPython);
				}

				else					
				{
					// Insert the string representation of value into the map
					PyObject* keyStrPython = PyObject_Str(key);
					PyObject* valueStrPython = PyObject_Str(value);
					std::string keyString   = PyString_AsString(keyStrPython);
					std::string valueString = PyString_AsString(valueStrPython);
					map->insert(keyString, valueString);
					Py_XDECREF(keyStrPython);
					Py_XDECREF(valueStrPython);
				}
			}

			Py_XDECREF(tupleType);
			Py_XDECREF(dictType);
			Py_XDECREF(tuple);
			Py_XDECREF(dict);
		}


		PythonInterpreter::PythonInterpreter()
		{
			// empty private ctor to prevent instanciation
			// TODO : change from static functions to interpreter factory with virtual interface
		}


		PythonInterpreter::~PythonInterpreter()
		{
			// empty private dtor to prevent instanciation
			// TODO : change from static functions to interpreter factory with virtual interface
		}

	}
}
