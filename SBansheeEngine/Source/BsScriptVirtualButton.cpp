#include "BsScriptVirtualButton.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ScriptVirtualButton::ScriptVirtualButton(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVirtualButton::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_InitVirtualButton", &ScriptVirtualButton::internal_InitVirtualButton);
	}

	UINT32 ScriptVirtualButton::internal_InitVirtualButton(MonoString* name)
	{
		String nameStr = MonoUtil::monoToString(name);

		VirtualButton vb(nameStr);
		return vb.buttonIdentifier;
	}

	MonoObject* ScriptVirtualButton::box(const VirtualButton& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return mono_value_box(MonoManager::instance().getDomain(),
			metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	VirtualButton ScriptVirtualButton::unbox(MonoObject* obj)
	{
		return *(VirtualButton*)mono_object_unbox(obj);
	}
}