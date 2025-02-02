#ifndef JSWINDOW_H_
#define JSWINDOW_H_

#define XP_UNIX

#include "jsapi.h"
#include "Window.h"

class JSWindow {
public:
	JSWindow() :
		m_pWindow(NULL) {
	}
	~JSWindow() {
		delete m_pWindow;
		m_pWindow = NULL;
	}
	static JSClass _class;

	static JSBool JSGetProperty(JSContext *cx, JSObject *obj, jsval id,
			jsval *vp);
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj, jsval id,
			jsval *vp);
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
			jsval *argv, jsval *rval);
	static void JSDestructor(JSContext *cx, JSObject *obj);

	static JSObject *JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);

	static JSPropertySpec _properties[];
	static JSFunctionSpec _methods[];
	
	// Property enum
	enum {title_prop};
	
	// Function enum
	
	

protected:
	void setWindow(Window *window) {
		m_pWindow = window;
	}
	Window* getWindow() {
		return m_pWindow;
	}
private:
	Window* m_pWindow;
};




#endif /*JSWINDOW_H_*/
