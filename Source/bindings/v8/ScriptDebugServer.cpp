/*
 * Copyright (c) 2010-2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "bindings/v8/ScriptDebugServer.h"

#include "DebuggerScriptSource.h"
#include "V8JavaScriptCallFrame.h"
#include "bindings/v8/ScopedPersistent.h"
#include "bindings/v8/ScriptObject.h"
#include "bindings/v8/V8Binding.h"
#include "bindings/v8/V8ScriptRunner.h"
#include "core/inspector/JavaScriptCallFrame.h"
#include "core/inspector/ScriptDebugListener.h"
#include "wtf/StdLibExtras.h"
#include "wtf/Vector.h"
#include "wtf/dtoa/utils.h"

namespace WebCore {

namespace {

class ClientDataImpl : public v8::Debug::ClientData {
public:
    ClientDataImpl(PassOwnPtr<ScriptDebugServer::Task> task) : m_task(task) { }
    virtual ~ClientDataImpl() { }
    ScriptDebugServer::Task* task() const { return m_task.get(); }
private:
    OwnPtr<ScriptDebugServer::Task> m_task;
};

}

v8::Local<v8::Value> ScriptDebugServer::callDebuggerMethod(const char* functionName, int argc, v8::Handle<v8::Value> argv[])
{
    v8::Handle<v8::Function> function = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::NewSymbol(functionName)));
    ASSERT(v8::Context::InContext());
    return V8ScriptRunner::callInternalFunction(function, m_debuggerScript.get(), argc, argv, m_isolate);
}

class ScriptDebugServer::ScriptPreprocessor {
    WTF_MAKE_NONCOPYABLE(ScriptPreprocessor);
public:
    ScriptPreprocessor(const String& preprocessorScript, v8::Isolate* isolate)
        : m_isolate(isolate)
    {
        v8::HandleScope scope(m_isolate);

        v8::Local<v8::Context> context = v8::Context::New(m_isolate);
        if (context.IsEmpty())
            return;
        v8::Context::Scope contextScope(context);

        String wrappedScript = "(" + preprocessorScript + ")";
        v8::Handle<v8::String> preprocessor = v8::String::New(wrappedScript.utf8().data(), wrappedScript.utf8().length());

        v8::Local<v8::Value> preprocessorFunction = V8ScriptRunner::compileAndRunInternalScript(preprocessor, m_isolate);
        if (preprocessorFunction.IsEmpty() || !preprocessorFunction->IsFunction())
            return;

        m_utilityContext.set(isolate, context);
        m_preprocessorFunction.set(isolate, v8::Handle<v8::Function>::Cast(preprocessorFunction));
    }

    String preprocessSourceCode(const String& sourceCode, const String& sourceName)
    {
        v8::HandleScope handleScope(m_isolate);

        if (m_preprocessorFunction.isEmpty())
            return sourceCode;

        v8::Local<v8::Context> context = v8::Local<v8::Context>::New(m_utilityContext.get());
        v8::Context::Scope contextScope(context);

        v8::Handle<v8::String> sourceCodeString = v8::String::New(sourceCode.utf8().data(), sourceCode.utf8().length());

        v8::Handle<v8::String> sourceNameString = v8::String::New(sourceName.utf8().data(), sourceName.utf8().length());
        v8::Handle<v8::Value> argv[] = { sourceCodeString, sourceNameString };
        v8::Handle<v8::Value> resultValue = V8ScriptRunner::callInternalFunction(m_preprocessorFunction.newLocal(m_isolate), context->Global(), WTF_ARRAY_LENGTH(argv), argv, m_isolate);

        if (!resultValue.IsEmpty() && resultValue->IsString()) {
            v8::String::Utf8Value utf8Value(resultValue);
            return String::fromUTF8(*utf8Value, utf8Value.length());
        }
        return sourceCode;
    }

    ~ScriptPreprocessor()
    {
    }

private:
    ScopedPersistent<v8::Context> m_utilityContext;
    String m_preprocessorBody;
    ScopedPersistent<v8::Function> m_preprocessorFunction;
    v8::Isolate* m_isolate;
};

ScriptDebugServer::ScriptDebugServer(v8::Isolate* isolate)
    : m_pauseOnExceptionsState(DontPauseOnExceptions)
    , m_breakpointsActivated(true)
    , m_runningNestedMessageLoop(false)
    , m_isolate(isolate)
{
}

ScriptDebugServer::~ScriptDebugServer()
{
}

String ScriptDebugServer::setBreakpoint(const String& sourceID, const ScriptBreakpoint& scriptBreakpoint, int* actualLineNumber, int* actualColumnNumber)
{
    v8::HandleScope scope;
    v8::Local<v8::Context> debuggerContext = v8::Debug::GetDebugContext();
    v8::Context::Scope contextScope(debuggerContext);

    v8::Local<v8::Object> args = v8::Object::New();
    args->Set(v8::String::NewSymbol("sourceID"), v8String(sourceID, debuggerContext->GetIsolate()));
    args->Set(v8::String::NewSymbol("lineNumber"), v8Integer(scriptBreakpoint.lineNumber, debuggerContext->GetIsolate()));
    args->Set(v8::String::NewSymbol("columnNumber"), v8Integer(scriptBreakpoint.columnNumber, debuggerContext->GetIsolate()));
    args->Set(v8::String::NewSymbol("condition"), v8String(scriptBreakpoint.condition, debuggerContext->GetIsolate()));

    v8::Handle<v8::Function> setBreakpointFunction = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::NewSymbol("setBreakpoint")));
    v8::Handle<v8::Value> breakpointId = v8::Debug::Call(setBreakpointFunction, args);
    if (!breakpointId->IsString())
        return "";
    *actualLineNumber = args->Get(v8::String::NewSymbol("lineNumber"))->Int32Value();
    *actualColumnNumber = args->Get(v8::String::NewSymbol("columnNumber"))->Int32Value();
    return toWebCoreString(breakpointId->ToString());
}

void ScriptDebugServer::removeBreakpoint(const String& breakpointId)
{
    v8::HandleScope scope;
    v8::Local<v8::Context> debuggerContext = v8::Debug::GetDebugContext();
    v8::Context::Scope contextScope(debuggerContext);

    v8::Local<v8::Object> args = v8::Object::New();
    args->Set(v8::String::NewSymbol("breakpointId"), v8String(breakpointId, debuggerContext->GetIsolate()));

    v8::Handle<v8::Function> removeBreakpointFunction = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::NewSymbol("removeBreakpoint")));
    v8::Debug::Call(removeBreakpointFunction, args);
}

void ScriptDebugServer::clearBreakpoints()
{
    ensureDebuggerScriptCompiled();
    v8::HandleScope scope;
    v8::Local<v8::Context> debuggerContext = v8::Debug::GetDebugContext();
    v8::Context::Scope contextScope(debuggerContext);

    v8::Handle<v8::Function> clearBreakpoints = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::NewSymbol("clearBreakpoints")));
    v8::Debug::Call(clearBreakpoints);
}

void ScriptDebugServer::setBreakpointsActivated(bool activated)
{
    ensureDebuggerScriptCompiled();
    v8::HandleScope scope;
    v8::Local<v8::Context> debuggerContext = v8::Debug::GetDebugContext();
    v8::Context::Scope contextScope(debuggerContext);

    v8::Local<v8::Object> args = v8::Object::New();
    args->Set(v8::String::NewSymbol("enabled"), v8::Boolean::New(activated));
    v8::Handle<v8::Function> setBreakpointsActivated = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::NewSymbol("setBreakpointsActivated")));
    v8::Debug::Call(setBreakpointsActivated, args);

    m_breakpointsActivated = activated;
}

ScriptDebugServer::PauseOnExceptionsState ScriptDebugServer::pauseOnExceptionsState()
{
    ensureDebuggerScriptCompiled();
    v8::HandleScope scope;
    v8::Context::Scope contextScope(v8::Debug::GetDebugContext());

    v8::Handle<v8::Value> argv[] = { v8Undefined() };
    v8::Handle<v8::Value> result = callDebuggerMethod("pauseOnExceptionsState", 0, argv);
    return static_cast<ScriptDebugServer::PauseOnExceptionsState>(result->Int32Value());
}

void ScriptDebugServer::setPauseOnExceptionsState(PauseOnExceptionsState pauseOnExceptionsState)
{
    ensureDebuggerScriptCompiled();
    v8::HandleScope scope;
    v8::Context::Scope contextScope(v8::Debug::GetDebugContext());

    v8::Handle<v8::Value> argv[] = { v8::Int32::New(pauseOnExceptionsState) };
    callDebuggerMethod("setPauseOnExceptionsState", 1, argv);
}

void ScriptDebugServer::setPauseOnNextStatement(bool pause)
{
    if (isPaused())
        return;
    if (pause)
        v8::Debug::DebugBreak(m_isolate);
    else
        v8::Debug::CancelDebugBreak(m_isolate);
}

bool ScriptDebugServer::canBreakProgram()
{
    if (!m_breakpointsActivated)
        return false;

    // FIXME: Remove this check once m_isolate->GetCurrentContext() does not crash.
    if (!v8::Context::InContext())
        return false;

    v8::HandleScope scope(m_isolate);
    return !m_isolate->GetCurrentContext().IsEmpty();
}

void ScriptDebugServer::breakProgram()
{
    if (!canBreakProgram())
        return;

    v8::HandleScope scope(m_isolate);
    if (m_breakProgramCallbackTemplate.get().IsEmpty()) {
        m_breakProgramCallbackTemplate.set(m_isolate, v8::FunctionTemplate::New());
        m_breakProgramCallbackTemplate.get()->SetCallHandler(&ScriptDebugServer::breakProgramCallback, v8::External::New(this));
    }

    m_pausedContext = m_isolate->GetCurrentContext();
    v8::Handle<v8::Function> breakProgramFunction = m_breakProgramCallbackTemplate.get()->GetFunction();
    v8::Debug::Call(breakProgramFunction);
    m_pausedContext.Clear();
}

void ScriptDebugServer::continueProgram()
{
    if (isPaused())
        quitMessageLoopOnPause();
    m_executionState.clear();
}

void ScriptDebugServer::stepIntoStatement()
{
    ASSERT(isPaused());
    v8::Handle<v8::Value> argv[] = { m_executionState.get() };
    callDebuggerMethod("stepIntoStatement", 1, argv);
    continueProgram();
}

void ScriptDebugServer::stepOverStatement()
{
    ASSERT(isPaused());
    v8::Handle<v8::Value> argv[] = { m_executionState.get() };
    callDebuggerMethod("stepOverStatement", 1, argv);
    continueProgram();
}

void ScriptDebugServer::stepOutOfFunction()
{
    ASSERT(isPaused());
    v8::Handle<v8::Value> argv[] = { m_executionState.get() };
    callDebuggerMethod("stepOutOfFunction", 1, argv);
    continueProgram();
}

bool ScriptDebugServer::setScriptSource(const String& sourceID, const String& newContent, bool preview, String* error, ScriptValue* newCallFrames, ScriptObject* result)
{
    class EnableLiveEditScope {
    public:
        EnableLiveEditScope() { v8::Debug::SetLiveEditEnabled(true); }
        ~EnableLiveEditScope() { v8::Debug::SetLiveEditEnabled(false); }
    };

    ensureDebuggerScriptCompiled();
    v8::HandleScope scope;

    OwnPtr<v8::Context::Scope> contextScope;
    v8::Handle<v8::Context> debuggerContext = v8::Debug::GetDebugContext();
    if (!isPaused())
        contextScope = adoptPtr(new v8::Context::Scope(debuggerContext));

    v8::Handle<v8::Value> argv[] = { v8String(sourceID, debuggerContext->GetIsolate()), v8String(newContent, debuggerContext->GetIsolate()), v8Boolean(preview) };

    v8::Local<v8::Value> v8result;
    {
        EnableLiveEditScope enableLiveEditScope;
        v8::TryCatch tryCatch;
        tryCatch.SetVerbose(false);
        v8result = callDebuggerMethod("liveEditScriptSource", 3, argv);
        if (tryCatch.HasCaught()) {
            v8::Local<v8::Message> message = tryCatch.Message();
            if (!message.IsEmpty())
                *error = toWebCoreStringWithUndefinedOrNullCheck(message->Get());
            else
                *error = "Unknown error.";
            return false;
        }
    }
    ASSERT(!v8result.IsEmpty());
    if (v8result->IsObject())
        *result = ScriptObject(ScriptState::current(), v8result->ToObject());

    // Call stack may have changed after if the edited function was on the stack.
    if (!preview && isPaused())
        *newCallFrames = currentCallFrame();
    return true;
}


void ScriptDebugServer::updateCallStack(ScriptValue* callFrame)
{
    if (isPaused())
        *callFrame = currentCallFrame();
}


void ScriptDebugServer::setScriptPreprocessor(const String& preprocessorBody)
{
    m_scriptPreprocessor.clear();
    if (!preprocessorBody.isEmpty())
        m_scriptPreprocessor = adoptPtr(new ScriptPreprocessor(preprocessorBody, m_isolate));
}

ScriptValue ScriptDebugServer::currentCallFrame()
{
    ASSERT(isPaused());
    v8::Handle<v8::Value> argv[] = { m_executionState.get() };
    v8::Handle<v8::Value> currentCallFrameV8 = callDebuggerMethod("currentCallFrame", 1, argv);

    ASSERT(!currentCallFrameV8.IsEmpty());
    if (!currentCallFrameV8->IsObject())
        return ScriptValue(v8::Null());

    RefPtr<JavaScriptCallFrame> currentCallFrame = JavaScriptCallFrame::create(v8::Debug::GetDebugContext(), v8::Handle<v8::Object>::Cast(currentCallFrameV8));
    v8::Context::Scope contextScope(m_pausedContext);
    return ScriptValue(toV8(currentCallFrame.release(), v8::Handle<v8::Object>(), m_pausedContext->GetIsolate()));
}

void ScriptDebugServer::interruptAndRun(PassOwnPtr<Task> task, v8::Isolate* isolate)
{
    v8::Debug::DebugBreakForCommand(new ClientDataImpl(task), isolate);
}

void ScriptDebugServer::runPendingTasks()
{
    v8::Debug::ProcessDebugMessages();
}

static ScriptDebugServer* toScriptDebugServer(v8::Handle<v8::Value> data)
{
    void* p = v8::Handle<v8::External>::Cast(data)->Value();
    return static_cast<ScriptDebugServer*>(p);
}

void ScriptDebugServer::breakProgramCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    ASSERT(2 == args.Length());
    
    ScriptDebugServer* thisPtr = toScriptDebugServer(args.Data());
    v8::Handle<v8::Value> exception;
    v8::Handle<v8::Array> hitBreakpoints;
    thisPtr->breakProgram(v8::Handle<v8::Object>::Cast(args[0]), exception, hitBreakpoints);
}

void ScriptDebugServer::breakProgram(v8::Handle<v8::Object> executionState, v8::Handle<v8::Value> exception, v8::Handle<v8::Array> hitBreakpointNumbers)
{
    // Don't allow nested breaks.
    if (isPaused())
        return;

    ScriptDebugListener* listener = getDebugListenerForContext(m_pausedContext);
    if (!listener)
        return;

    Vector<String> breakpointIds;
    if (!hitBreakpointNumbers.IsEmpty()) {
        breakpointIds.resize(hitBreakpointNumbers->Length());
        for (size_t i = 0; i < hitBreakpointNumbers->Length(); i++)
            breakpointIds[i] = toWebCoreStringWithUndefinedOrNullCheck(hitBreakpointNumbers->Get(i));
    }

    m_executionState.set(m_isolate, executionState);
    ScriptState* currentCallFrameState = ScriptState::forContext(m_pausedContext);
    listener->didPause(currentCallFrameState, currentCallFrame(), ScriptValue(exception), breakpointIds);

    m_runningNestedMessageLoop = true;
    runMessageLoopOnPause(m_pausedContext);
    m_runningNestedMessageLoop = false;
}

void ScriptDebugServer::breakProgram(const v8::Debug::EventDetails& eventDetails, v8::Handle<v8::Value> exception, v8::Handle<v8::Array> hitBreakpointNumbers)
{
    m_pausedContext = eventDetails.GetEventContext();
    breakProgram(eventDetails.GetExecutionState(), exception, hitBreakpointNumbers);
    m_pausedContext.Clear();
}

void ScriptDebugServer::v8DebugEventCallback(const v8::Debug::EventDetails& eventDetails)
{
    ScriptDebugServer* thisPtr = toScriptDebugServer(eventDetails.GetCallbackData());
    thisPtr->handleV8DebugEvent(eventDetails);
}

void ScriptDebugServer::handleV8DebugEvent(const v8::Debug::EventDetails& eventDetails)
{
    v8::DebugEvent event = eventDetails.GetEvent();

    if (event == v8::BreakForCommand) {
        ClientDataImpl* data = static_cast<ClientDataImpl*>(eventDetails.GetClientData());
        data->task()->run();
        return;
    }

    if (event != v8::Break && event != v8::Exception && event != v8::AfterCompile && event != v8::BeforeCompile)
        return;

    v8::Handle<v8::Context> eventContext = eventDetails.GetEventContext();
    ASSERT(!eventContext.IsEmpty());

    ScriptDebugListener* listener = getDebugListenerForContext(eventContext);
    if (listener) {
        v8::HandleScope scope;
        v8::Local<v8::Context> debugContext = v8::Debug::GetDebugContext();
        v8::Isolate* isolate = debugContext->GetIsolate();
        if (event == v8::BeforeCompile) {

            if (!m_scriptPreprocessor)
                return;

            OwnPtr<ScriptPreprocessor> preprocessor(m_scriptPreprocessor.release());
            v8::Context::Scope contextScope(debugContext);
            v8::Handle<v8::Function> getScriptSourceFunction = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::New("getScriptSource")));
            v8::Handle<v8::Value> argv[] = { eventDetails.GetEventData() };
            v8::Handle<v8::Value> script = V8ScriptRunner::callInternalFunction(getScriptSourceFunction, m_debuggerScript.get(), WTF_ARRAY_LENGTH(argv), argv, isolate);

            v8::Handle<v8::Function> getScriptNameFunction = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::New("getScriptName")));
            v8::Handle<v8::Value> argv1[] = { eventDetails.GetEventData() };
            v8::Handle<v8::Value> scriptName = V8ScriptRunner::callInternalFunction(getScriptNameFunction, m_debuggerScript.get(), WTF_ARRAY_LENGTH(argv1), argv1, isolate);
            v8::Handle<v8::Function> setScriptSourceFunction = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::New("setScriptSource")));
            String patchedScript = preprocessor->preprocessSourceCode(toWebCoreStringWithUndefinedOrNullCheck(script), toWebCoreStringWithUndefinedOrNullCheck(scriptName));

            v8::Handle<v8::Value> argv2[] = { eventDetails.GetEventData(), v8String(patchedScript, isolate) };
            V8ScriptRunner::callInternalFunction(setScriptSourceFunction, m_debuggerScript.get(), WTF_ARRAY_LENGTH(argv2), argv2, isolate);
            m_scriptPreprocessor = preprocessor.release();
        } else if (event == v8::AfterCompile) {
            v8::Context::Scope contextScope(v8::Debug::GetDebugContext());
            v8::Handle<v8::Function> getAfterCompileScript = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::NewSymbol("getAfterCompileScript")));
            v8::Handle<v8::Value> argv[] = { eventDetails.GetEventData() };
            v8::Handle<v8::Value> value = V8ScriptRunner::callInternalFunction(getAfterCompileScript, m_debuggerScript.get(), WTF_ARRAY_LENGTH(argv), argv, isolate);
            ASSERT(value->IsObject());
            v8::Handle<v8::Object> object = v8::Handle<v8::Object>::Cast(value);
            dispatchDidParseSource(listener, object);
        } else if (event == v8::Exception) {
            v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(1);
            // Stack trace is empty in case of syntax error. Silently continue execution in such cases.
            if (!stackTrace->GetFrameCount())
                return;
            v8::Handle<v8::Object> eventData = eventDetails.GetEventData();
            v8::Handle<v8::Value> exceptionGetterValue = eventData->Get(v8::String::NewSymbol("exception"));
            ASSERT(!exceptionGetterValue.IsEmpty() && exceptionGetterValue->IsFunction());
            v8::Handle<v8::Value> exception = V8ScriptRunner::callInternalFunction(v8::Handle<v8::Function>::Cast(exceptionGetterValue), eventData, 0, 0, isolate);
            breakProgram(eventDetails, exception, v8::Handle<v8::Array>());
        } else if (event == v8::Break) {
            v8::Handle<v8::Function> getBreakpointNumbersFunction = v8::Local<v8::Function>::Cast(m_debuggerScript.get()->Get(v8::String::NewSymbol("getBreakpointNumbers")));
            v8::Handle<v8::Value> argv[] = { eventDetails.GetEventData() };
            v8::Handle<v8::Value> hitBreakpoints = V8ScriptRunner::callInternalFunction(getBreakpointNumbersFunction, m_debuggerScript.get(), WTF_ARRAY_LENGTH(argv), argv, isolate);
            ASSERT(hitBreakpoints->IsArray());

            breakProgram(eventDetails, v8::Handle<v8::Value>(), hitBreakpoints.As<v8::Array>());
        }
    }
}

void ScriptDebugServer::dispatchDidParseSource(ScriptDebugListener* listener, v8::Handle<v8::Object> object)
{
    String sourceID = toWebCoreStringWithUndefinedOrNullCheck(object->Get(v8::String::NewSymbol("id")));

    ScriptDebugListener::Script script;
    script.url = toWebCoreStringWithUndefinedOrNullCheck(object->Get(v8::String::NewSymbol("name")));
    script.source = toWebCoreStringWithUndefinedOrNullCheck(object->Get(v8::String::NewSymbol("source")));
    script.sourceMappingURL = toWebCoreStringWithUndefinedOrNullCheck(object->Get(v8::String::NewSymbol("sourceMappingURL")));
    script.startLine = object->Get(v8::String::NewSymbol("startLine"))->ToInteger()->Value();
    script.startColumn = object->Get(v8::String::NewSymbol("startColumn"))->ToInteger()->Value();
    script.endLine = object->Get(v8::String::NewSymbol("endLine"))->ToInteger()->Value();
    script.endColumn = object->Get(v8::String::NewSymbol("endColumn"))->ToInteger()->Value();
    script.isContentScript = object->Get(v8::String::NewSymbol("isContentScript"))->ToBoolean()->Value();

    listener->didParseSource(sourceID, script);
}

void ScriptDebugServer::ensureDebuggerScriptCompiled()
{
    if (!m_debuggerScript.get().IsEmpty())
        return;

    v8::HandleScope scope(m_isolate);
    v8::Context::Scope contextScope(v8::Debug::GetDebugContext());
    v8::Handle<v8::String> source = v8String(String(reinterpret_cast<const char*>(DebuggerScriptSource_js), sizeof(DebuggerScriptSource_js)), m_isolate);
    v8::Local<v8::Value> value = V8ScriptRunner::compileAndRunInternalScript(source, m_isolate);
    ASSERT(!value.IsEmpty());
    ASSERT(value->IsObject());
    m_debuggerScript.set(m_isolate, v8::Handle<v8::Object>::Cast(value));
}

v8::Local<v8::Value> ScriptDebugServer::functionScopes(v8::Handle<v8::Function> function)
{
    ensureDebuggerScriptCompiled();

    v8::Handle<v8::Value> argv[] = { function };
    return callDebuggerMethod("getFunctionScopes", 1, argv);
}

v8::Local<v8::Value> ScriptDebugServer::getInternalProperties(v8::Handle<v8::Object>& object)
{
    if (m_debuggerScript.get().IsEmpty())
        return v8::Local<v8::Value>::New(m_isolate, v8::Undefined());

    v8::Handle<v8::Value> argv[] = { object };
    return callDebuggerMethod("getInternalProperties", 1, argv);
}

v8::Handle<v8::Value> ScriptDebugServer::setFunctionVariableValue(v8::Handle<v8::Value> functionValue, int scopeNumber, const String& variableName, v8::Handle<v8::Value> newValue)
{
    v8::Local<v8::Context> debuggerContext = v8::Debug::GetDebugContext();
    if (m_debuggerScript.get().IsEmpty())
        return v8::ThrowException(v8::String::New("Debugging is not enabled."));

    v8::Handle<v8::Value> argv[] = {
        functionValue,
        v8::Handle<v8::Value>(v8::Integer::New(scopeNumber)),
        v8String(variableName, debuggerContext->GetIsolate()),
        newValue
    };
    return callDebuggerMethod("setFunctionVariableValue", 4, argv);
}


bool ScriptDebugServer::isPaused()
{
    return !m_executionState.get().IsEmpty();
}

void ScriptDebugServer::compileScript(ScriptState* state, const String& expression, const String& sourceURL, String* scriptId, String* exceptionMessage)
{
    v8::HandleScope handleScope;
    v8::Handle<v8::Context> context = state->context();
    if (context.IsEmpty())
        return;
    v8::Isolate* isolate = context->GetIsolate();
    v8::Context::Scope contextScope(context);

    v8::Handle<v8::String> source = v8String(expression, isolate);
    v8::TryCatch tryCatch;
    v8::Local<v8::Script> script = V8ScriptRunner::compileScript(source, sourceURL, TextPosition(), 0, isolate);
    if (tryCatch.HasCaught()) {
        v8::Local<v8::Message> message = tryCatch.Message();
        if (!message.IsEmpty())
            *exceptionMessage = toWebCoreStringWithUndefinedOrNullCheck(message->Get());
        return;
    }
    if (script.IsEmpty())
        return;

    *scriptId = toWebCoreStringWithUndefinedOrNullCheck(script->Id());
    m_compiledScripts.set(*scriptId, adoptPtr(new ScopedPersistent<v8::Script>(script)));
}

void ScriptDebugServer::clearCompiledScripts()
{
    m_compiledScripts.clear();
}

void ScriptDebugServer::runScript(ScriptState* state, const String& scriptId, ScriptValue* result, bool* wasThrown, String* exceptionMessage)
{
    if (!m_compiledScripts.contains(scriptId))
        return;
    v8::HandleScope handleScope;
    ScopedPersistent<v8::Script>* scriptHandle = m_compiledScripts.get(scriptId);
    v8::Local<v8::Script> script = v8::Local<v8::Script>::New(scriptHandle->get());
    m_compiledScripts.remove(scriptId);
    if (script.IsEmpty())
        return;

    v8::Handle<v8::Context> context = state->context();
    if (context.IsEmpty())
        return;
    v8::Context::Scope contextScope(context);
    v8::TryCatch tryCatch;
    v8::Local<v8::Value> value = V8ScriptRunner::runCompiledScript(script, state->scriptExecutionContext());
    *wasThrown = false;
    if (tryCatch.HasCaught()) {
        *wasThrown = true;
        *result = ScriptValue(tryCatch.Exception());
        v8::Local<v8::Message> message = tryCatch.Message();
        if (!message.IsEmpty())
            *exceptionMessage = toWebCoreStringWithUndefinedOrNullCheck(message->Get());
    } else
        *result = ScriptValue(value);
}

} // namespace WebCore
