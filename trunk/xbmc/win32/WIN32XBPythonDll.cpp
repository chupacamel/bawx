/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "../lib/libPython/XBPythonDll.h"
#include "../../cores/DllLoader/DllLoader.h"

template<byte count>
struct SVaPassNext{
  SVaPassNext<count-1> big;
  DWORD dw;
};
template<> struct SVaPassNext<0>{};
//SVaPassNext - is generator of structure of any size at compile time.

class CVaPassNext{
  public:
    SVaPassNext<50> svapassnext;
    CVaPassNext(va_list & args){
      try{//to avoid access violation
        memcpy(&svapassnext, args, sizeof(svapassnext));
      } catch (...) {}
    }
};
#define va_pass(valist) CVaPassNext(valist).svapassnext

// macro to make life a bit simpler

#define DLL_FUNCTION(function) #function, (void**)&p_##function
#define DLL_VA_FUNCTION(function) #function, (void**)&p_va_##function

// use these if the dll exports pointers
#define DLL_POINTER_DATA(data) #data, (void**)&data
#define DATA_POINTER(data) unsigned long data;

// use these if the dll exports data structs
#define DLL_OBJECT_DATA(data) #data, (void**)&pointer_##data

#define _Py_NoneStruct (*((PyObject*)pointer__Py_NoneStruct))
#define PyString_Type (*((PyTypeObject*)pointer_PyString_Type))
#define PyList_Type (*((PyTypeObject*)pointer_PyList_Type))
#define PyLong_Type (*((PyTypeObject*)pointer_PyLong_Type))
#define PyInt_Type (*((PyTypeObject*)pointer_PyInt_Type))
#define PyUnicode_Type (*((PyTypeObject*)pointer_PyUnicode_Type))
#define PyTuple_Type (*((PyTypeObject*)pointer_PyTuple_Type))
#define PyDict_Type (*((PyTypeObject*)pointer_PyDict_Type))
#define PySlice_Type (*((PyTypeObject*)pointer_PySlice_Type))

#define DLL_ORD_FUNCTION(ord, function) "hapdbg.dll", ord, (void**)&p_##function

#define FUNCTION(function) \
    void* (__cdecl* p_##function)(); \
    void* function() { return p_##function(); }

#define FUNCTION4(function) \
    void* (__cdecl* p_##function)(void* a); \
    void* function(void* a) { return p_##function(a); }

#define FUNCTION8(function) \
    void* (__cdecl* p_##function)(void* a, void* b); \
    void* function(void* a, void* b) { return p_##function(a, b); }

#define VA_FUNC_START(x) \
      void* ret; \
      va_list va; \
      va_start(va, x); \
      ret =

#define VA_FUNC_END \
      va_end(va); \
      return ret;

#define VA_FUNCTION8(function) \
    void* (__cdecl* p_va_##function)(void* a, ...); \
    void* function(void* a, ...) { VA_FUNC_START(a) p_va_##function(a, va_pass(va)); VA_FUNC_END }

#define VA_FUNCTION12(function) \
    void* (__cdecl* p_va_##function)(void* a, void* b, ...); \
    void* function(void* a, void* b, ...) { VA_FUNC_START(b) p_va_##function(a, b, va_pass(va)); VA_FUNC_END }

#define VA_FUNCTION16(function) \
    void* (__cdecl* p_va_##function)(void* a, void* b, void* c, ...); \
    void* function(void* a, void* b, void* c, ...) { VA_FUNC_START(c) p_va_##function(a, b, c, va_pass(va)); VA_FUNC_END }

#define VA_FUNCTION20(function) \
    void* (__cdecl* p_va_##function)(void* a, void* b, void* c, void* d, ...); \
    void* function(void* a, void* b, void* c, void* d, ...) { VA_FUNC_START(d) p_va_##function(a, b, c, d, va_pass(va)); VA_FUNC_END }

#define FUNCTION12(function) \
    void* (__cdecl* p_##function)(void* a, void* b, void* c); \
    void* function(void* a, void* b, void* c) { return p_##function(a, b, c); }

#define FUNCTION16(function) \
    void* (__cdecl* p_##function)(void* a, void* b, void* c, void* d); \
    void* function(void* a, void* b, void* c, void* d) { return p_##function(a, b, c, d); }

#define FUNCTION20(function) \
    void* (__cdecl* p_##function)(void* a, void* b, void* c, void* d, void* e); \
    void* function(void* a, void* b, void* c, void* d, void* e) { return p_##function(a, b, c, d, e); }

extern "C"
{
  /*****************************************
   * python24.dll
   */

  FUNCTION(PyEval_ReleaseLock)
  FUNCTION(PyEval_AcquireLock)
  FUNCTION(PyThreadState_Get)
  FUNCTION4(PyRun_SimpleString)
  FUNCTION(PyEval_InitThreads)
  FUNCTION(PyEval_ThreadsInitialized)
  FUNCTION(Py_Initialize)
  FUNCTION(Py_IsInitialized)
  FUNCTION(Py_Finalize)
  FUNCTION(Py_NewInterpreter)
  FUNCTION4(Py_EndInterpreter)
  FUNCTION4(PyThreadState_Swap)
  FUNCTION8(PyErr_SetString)
  FUNCTION4(PyThreadState_New)
  FUNCTION(PyErr_Print)
  FUNCTION(PyErr_Occurred)
  FUNCTION8(PyRun_SimpleFile)
  FUNCTION4(PySys_SetPath)
  FUNCTION16(PyRun_String)
  FUNCTION4(PySys_GetObject)
  FUNCTION8(PySys_SetObject)

#ifdef _LINUX
  FUNCTION(Py_GetPath)
#endif
  FUNCTION4(PyThreadState_Delete)
  FUNCTION4(PyThreadState_Clear)
  VA_FUNCTION8(Py_BuildValue)
  VA_FUNCTION12(PyArg_Parse)
  VA_FUNCTION12(PyArg_ParseTuple)
  FUNCTION8(PyType_IsSubtype)
  VA_FUNCTION20(PyArg_ParseTupleAndKeywords)
  FUNCTION4(PyString_AsString)
  FUNCTION8(Py_AddPendingCall)
  FUNCTION8(PyList_GetItem)
  VA_FUNCTION16(PyObject_CallMethod)
  FUNCTION4(PyList_Size)
  FUNCTION4(_PyObject_New)
  FUNCTION4(PyLong_AsLong)
  FUNCTION4(PyLong_AsLongLong)
  VA_FUNCTION12(PyErr_Format)
#ifndef _LINUX
  FUNCTION4(PyUnicodeUCS2_AsUnicode)
#else
  FUNCTION4(PyUnicodeUCS4_AsUnicode)
#endif
  FUNCTION(Py_MakePendingCalls)
  FUNCTION(PyEval_SaveThread)
  FUNCTION4(PyEval_RestoreThread)
  FUNCTION4(PyLong_FromLong)
  FUNCTION12(PyModule_AddStringConstant)
  FUNCTION12(PyModule_AddObject)
  FUNCTION20(Py_InitModule4)
  FUNCTION4(PyInt_AsLong)
  FUNCTION4(PyFloat_AsDouble)
  FUNCTION4(PyString_FromString)
  FUNCTION12(PyModule_AddIntConstant)
  VA_FUNCTION12(PyObject_CallMethodObjArgs)
  VA_FUNCTION12(PyObject_CallFunction) // va arg
  FUNCTION12(PyDict_SetItemString)

  FUNCTION(PyDict_New)
  FUNCTION4(PyModule_GetDict)
  FUNCTION4(PyImport_Import)

  FUNCTION4(PyFloat_FromDouble)
  FUNCTION8(PyObject_GetAttrString)
  FUNCTION4(_PyObject_GetDictPtr)
  FUNCTION12(PyTuple_SetItem)
  FUNCTION12(PyDict_SetItem)
  FUNCTION(PyErr_Clear)
  FUNCTION8(PyDict_GetItem)
  VA_FUNCTION8(PyString_FromFormat)
  FUNCTION8(PyObject_GetAttr)
  FUNCTION12(PyObject_Call)
  FUNCTION8(PyObject_Init) 
  FUNCTION4(PyObject_Size)
  FUNCTION8(PyString_Format)
  FUNCTION4(PyTuple_New)
  FUNCTION8(PyString_ConcatAndDel)
  FUNCTION8(PyCObject_Import)
  FUNCTION4(PyObject_IsTrue)
  FUNCTION8(PyObject_GenericGetAttr)
  FUNCTION8(PyInstance_NewRaw)
  FUNCTION8(_PyInstance_Lookup)
  FUNCTION4(PyBool_FromLong)
  FUNCTION8(PyCObject_FromVoidPtr)
  FUNCTION4(PyLong_FromVoidPtr)
  VA_FUNCTION20(PyArg_UnpackTuple)    
  VA_FUNCTION8(PyObject_CallFunctionObjArgs)    
  FUNCTION4(PyObject_Malloc)    
  FUNCTION4(PyObject_Free)   
  FUNCTION8(PyString_FromStringAndSize)
  FUNCTION12(PyString_AsStringAndSize)
  FUNCTION12(PyObject_SetAttr)
  FUNCTION(PyGILState_Ensure)
  FUNCTION4(PyGILState_Release)  
  FUNCTION8(PyErr_SetObject)
  FUNCTION4(PyLong_AsUnsignedLong)
  FUNCTION4(PySequence_Size)
  FUNCTION12(PyErr_Fetch)
  FUNCTION4(PySequence_Check)
  FUNCTION8(PySequence_GetItem)
  FUNCTION4(PyObject_Str)
  FUNCTION4(PyLong_FromUnsignedLong)
  FUNCTION4(PyList_New)
  FUNCTION4(PyLong_AsDouble)  
  //FUNCTION12(PyDict_SetItemString) // duplicate
  FUNCTION4(PyImport_AddModule)
  
  //VA_FUNCTION16(PyObject_CallMethod) // already exists

  
  FUNCTION4(PyInt_FromLong)
  FUNCTION8(PyDict_GetItemString)
  //FUNCTION8(PyDict_GetItem) duplicate
  FUNCTION4(PyDict_Keys)
  FUNCTION16(PyDict_Next)
  FUNCTION4(PyDict_Size)
  FUNCTION4(PyType_Ready)
  FUNCTION12(PyType_GenericNew)
  FUNCTION8(PySys_SetArgv)
  FUNCTION12(PyObject_RichCompare)
  FUNCTION12(PySlice_New)
  FUNCTION20(PySlice_GetIndices)
  FUNCTION8(PyThreadState_SetAsyncExc)

  // PyFloat_FromDouble(double)
  //void* (__cdecl* p_PyFloat_FromDouble)(double a); 
  //void* PyFloat_FromDouble(double a) { return p_PyFloat_FromDouble(a); }

  DATA_OBJECT(PyExc_SystemError)
  DATA_OBJECT(PyExc_ValueError)
  DATA_OBJECT(PyExc_Exception)
  DATA_OBJECT(PyExc_TypeError)
  DATA_OBJECT(PyExc_KeyboardInterrupt)
  DATA_OBJECT(PyExc_RuntimeError)
  DATA_OBJECT(PyExc_ReferenceError)
  DATA_OBJECT(PyExc_OverflowError)
  DATA_OBJECT(PyExc_NotImplementedError)
  DATA_OBJECT(PyExc_IndexError)
  DATA_OBJECT(PyExc_AttributeError)
  DATA_OBJECT(PyExc_IOError)
  DATA_OBJECT(PyExc_SyntaxError)
  DATA_OBJECT(PyExc_ZeroDivisionError)
  DATA_OBJECT(PyExc_MemoryError)
  DATA_OBJECT(PyExc_StopIteration)
  DATA_OBJECT(PyClass_Type)
  DATA_OBJECT(_PyWeakref_CallableProxyType)
  DATA_OBJECT(PyType_Type)
  DATA_OBJECT(_PyWeakref_ProxyType)
  DATA_OBJECT(PyInstance_Type)
  DATA_OBJECT(_Py_NoneStruct)
  DATA_OBJECT(_Py_NotImplementedStruct)
  DATA_OBJECT(_Py_TrueStruct)
  DATA_OBJECT(_Py_ZeroStruct)
  DATA_OBJECT(PyString_Type)
  DATA_OBJECT(PyList_Type)
  DATA_OBJECT(PyLong_Type)
  DATA_OBJECT(PyInt_Type)
  DATA_OBJECT(PyUnicode_Type)
  DATA_OBJECT(PyTuple_Type)
  DATA_OBJECT(PyDict_Type)
  DATA_OBJECT(PyFloat_Type)
  DATA_OBJECT(PySlice_Type)
  DATA_OBJECT(PyExc_SystemExit)

  bool python_load_dll(LibraryLoader& dll)
  {
    bool bResult;

    bResult = (
      dll.ResolveExport(DLL_FUNCTION(PyEval_ReleaseLock)) &&
      dll.ResolveExport(DLL_FUNCTION(PyEval_AcquireLock)) &&
      dll.ResolveExport(DLL_FUNCTION(PyThreadState_Get)) &&
      dll.ResolveExport(DLL_FUNCTION(PyRun_SimpleString)) &&
      dll.ResolveExport(DLL_FUNCTION(PyEval_InitThreads)) &&
      dll.ResolveExport(DLL_FUNCTION(PyEval_ThreadsInitialized)) &&
      dll.ResolveExport(DLL_FUNCTION(Py_Initialize)) &&
      dll.ResolveExport(DLL_FUNCTION(Py_IsInitialized)) &&
      dll.ResolveExport(DLL_FUNCTION(Py_Finalize)) &&
      dll.ResolveExport(DLL_FUNCTION(Py_NewInterpreter)) &&
      dll.ResolveExport(DLL_FUNCTION(Py_EndInterpreter)) &&
      dll.ResolveExport(DLL_FUNCTION(PyThreadState_Swap)) &&
      dll.ResolveExport(DLL_FUNCTION(PyErr_SetString)) &&
      dll.ResolveExport(DLL_FUNCTION(PyThreadState_New)) &&
      dll.ResolveExport(DLL_FUNCTION(PyErr_Print)) &&
      dll.ResolveExport(DLL_FUNCTION(PyErr_Occurred)) &&
      dll.ResolveExport(DLL_FUNCTION(PyRun_SimpleFile)) &&
      dll.ResolveExport(DLL_FUNCTION(PySys_SetPath)) &&
      dll.ResolveExport(DLL_FUNCTION(PyRun_String)) &&
      dll.ResolveExport(DLL_FUNCTION(PySys_GetObject)) &&
      dll.ResolveExport(DLL_FUNCTION(PySys_SetObject)) &&
#ifdef _LINUX
      dll.ResolveExport(DLL_FUNCTION(Py_GetPath)) &&
#endif
      dll.ResolveExport(DLL_FUNCTION(PyThreadState_Delete)) &&
      dll.ResolveExport(DLL_FUNCTION(PyThreadState_Clear)) &&
      dll.ResolveExport(DLL_VA_FUNCTION(Py_BuildValue)) &&
	  dll.ResolveExport(DLL_VA_FUNCTION(PyArg_Parse)) &&
	  dll.ResolveExport(DLL_VA_FUNCTION(PyArg_ParseTuple)) &&
      dll.ResolveExport(DLL_FUNCTION(PyType_IsSubtype)) &&
      dll.ResolveExport(DLL_VA_FUNCTION(PyArg_ParseTupleAndKeywords)) &&
      dll.ResolveExport(DLL_FUNCTION(PyString_AsString)) &&
      dll.ResolveExport(DLL_FUNCTION(Py_AddPendingCall)) &&
	  dll.ResolveExport(DLL_FUNCTION(PyList_GetItem)) &&
      dll.ResolveExport(DLL_VA_FUNCTION(PyObject_CallMethod)) &&
      dll.ResolveExport(DLL_FUNCTION(PyList_Size)) &&
      dll.ResolveExport(DLL_FUNCTION(_PyObject_New)) &&
      dll.ResolveExport(DLL_FUNCTION(PyLong_AsLong)) &&
      dll.ResolveExport(DLL_FUNCTION(PyLong_AsLongLong)) &&
      dll.ResolveExport(DLL_VA_FUNCTION(PyErr_Format)) &&
#ifndef _LINUX
      dll.ResolveExport(DLL_FUNCTION(PyUnicodeUCS2_AsUnicode)) &&
#else
      dll.ResolveExport(DLL_FUNCTION(PyUnicodeUCS4_AsUnicode)) &&
#endif
      dll.ResolveExport(DLL_FUNCTION(Py_MakePendingCalls)) &&
      dll.ResolveExport(DLL_FUNCTION(PyEval_SaveThread)) &&
      dll.ResolveExport(DLL_FUNCTION(PyEval_RestoreThread)) &&
      dll.ResolveExport(DLL_FUNCTION(PyLong_FromLong)) &&
      dll.ResolveExport(DLL_FUNCTION(PyModule_AddStringConstant)) &&
      dll.ResolveExport(DLL_FUNCTION(PyModule_AddObject)) &&
      dll.ResolveExport(DLL_FUNCTION(Py_InitModule4)) &&
      dll.ResolveExport(DLL_FUNCTION(PyInt_AsLong)) &&
      dll.ResolveExport(DLL_FUNCTION(PyFloat_AsDouble)) &&
      dll.ResolveExport(DLL_FUNCTION(PyString_FromString)) &&
      dll.ResolveExport(DLL_FUNCTION(PyModule_AddIntConstant)) &&
	  dll.ResolveExport(DLL_VA_FUNCTION(PyObject_CallMethodObjArgs))&&
      dll.ResolveExport(DLL_VA_FUNCTION(PyObject_CallFunction)) &&
	  dll.ResolveExport(DLL_FUNCTION(PyDict_SetItemString)) &&
      dll.ResolveExport(DLL_FUNCTION(PyDict_New)) &&
      dll.ResolveExport(DLL_FUNCTION(PyModule_GetDict)) &&
      dll.ResolveExport(DLL_FUNCTION(PyImport_Import)) &&
      dll.ResolveExport(DLL_FUNCTION(PyFloat_FromDouble)) &&
      dll.ResolveExport(DLL_FUNCTION(PyInt_FromLong)) &&
      dll.ResolveExport(DLL_FUNCTION(PyDict_GetItemString)) &&
      dll.ResolveExport(DLL_FUNCTION(PyDict_GetItem)) &&
      dll.ResolveExport(DLL_FUNCTION(PyDict_Keys)) &&
      dll.ResolveExport(DLL_FUNCTION(PyDict_Next)) &&
      dll.ResolveExport(DLL_FUNCTION(PyDict_Size)) &&
      dll.ResolveExport(DLL_FUNCTION(PyType_Ready)) &&
      dll.ResolveExport(DLL_FUNCTION(PyType_GenericNew)) &&
      dll.ResolveExport(DLL_FUNCTION(PySys_SetArgv)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_RichCompare)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_GetAttrString)) && //
      dll.ResolveExport(DLL_FUNCTION(_PyObject_GetDictPtr)) && //
      dll.ResolveExport(DLL_FUNCTION(PyTuple_SetItem)) && //
      dll.ResolveExport(DLL_FUNCTION(PyDict_SetItem)) && //
      dll.ResolveExport(DLL_FUNCTION(PyErr_Clear)) &&
      dll.ResolveExport(DLL_VA_FUNCTION(PyString_FromFormat)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_GetAttr)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_Call)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_Init)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_Size)) &&
      dll.ResolveExport(DLL_FUNCTION(PyString_Format)) &&
      dll.ResolveExport(DLL_FUNCTION(PyTuple_New)) &&
      dll.ResolveExport(DLL_FUNCTION(PyString_ConcatAndDel)) &&
      dll.ResolveExport(DLL_FUNCTION(PyCObject_Import)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_IsTrue)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_GenericGetAttr)) &&
      dll.ResolveExport(DLL_FUNCTION(PyInstance_NewRaw)) &&
      dll.ResolveExport(DLL_FUNCTION(_PyInstance_Lookup)) &&
      dll.ResolveExport(DLL_FUNCTION(PyBool_FromLong)) &&
      dll.ResolveExport(DLL_FUNCTION(PyCObject_FromVoidPtr)) &&
      dll.ResolveExport(DLL_FUNCTION(PyLong_FromVoidPtr)) &&
      dll.ResolveExport(DLL_VA_FUNCTION(PyArg_UnpackTuple)) &&
      dll.ResolveExport(DLL_VA_FUNCTION(PyObject_CallFunctionObjArgs)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_Free)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_Malloc)) &&
      dll.ResolveExport(DLL_FUNCTION(PyString_FromStringAndSize)) &&
      dll.ResolveExport(DLL_FUNCTION(PyString_AsStringAndSize)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_SetAttr)) &&
      dll.ResolveExport(DLL_FUNCTION(PyGILState_Ensure)) &&
      dll.ResolveExport(DLL_FUNCTION(PyGILState_Release)) &&
      dll.ResolveExport(DLL_FUNCTION(PyErr_SetObject)) &&
      dll.ResolveExport(DLL_FUNCTION(PyLong_AsUnsignedLong)) &&
      dll.ResolveExport(DLL_FUNCTION(PySequence_Size)) &&
      dll.ResolveExport(DLL_FUNCTION(PyErr_Fetch)) &&
      dll.ResolveExport(DLL_FUNCTION(PySequence_Check)) &&
      dll.ResolveExport(DLL_FUNCTION(PySequence_GetItem)) &&
      dll.ResolveExport(DLL_FUNCTION(PyObject_Str)) &&
      dll.ResolveExport(DLL_FUNCTION(PyLong_FromUnsignedLong)) &&
      dll.ResolveExport(DLL_FUNCTION(PyList_New)) &&
      dll.ResolveExport(DLL_FUNCTION(PyLong_AsDouble)) &&
	  //dll.ResolveExport(DLL_FUNCTION(PyDict_SetItemString)) && // duplicate
      dll.ResolveExport(DLL_FUNCTION(PyImport_AddModule)) &&
	  dll.ResolveExport(DLL_FUNCTION(PySlice_GetIndices)) &&
	  dll.ResolveExport(DLL_FUNCTION(PySlice_New)) &&
    dll.ResolveExport(DLL_FUNCTION(PyThreadState_SetAsyncExc)) &&

	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_SystemError)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyExc_ValueError)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyExc_Exception)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyExc_TypeError)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyExc_KeyboardInterrupt)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyExc_RuntimeError)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyExc_ReferenceError)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(_Py_NoneStruct)) && // ??
      dll.ResolveExport(DLL_OBJECT_DATA(_Py_NotImplementedStruct)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(_Py_TrueStruct)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(_Py_ZeroStruct)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyString_Type)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyList_Type)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyLong_Type)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyInt_Type)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyUnicode_Type)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyTuple_Type)) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyDict_Type))) &&
      dll.ResolveExport(DLL_OBJECT_DATA(PyFloat_Type)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PySlice_Type)) &&
	  
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_OverflowError)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_NotImplementedError)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_IndexError)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_AttributeError)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_IOError)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_SyntaxError)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_ZeroDivisionError)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_MemoryError)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyExc_StopIteration)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyClass_Type)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(_PyWeakref_CallableProxyType)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyType_Type)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(_PyWeakref_ProxyType)) &&
	  dll.ResolveExport(DLL_OBJECT_DATA(PyInstance_Type)) &&
    dll.ResolveExport(DLL_OBJECT_DATA(PyExc_SystemExit));
	  
      

    return bResult;
  }
}
