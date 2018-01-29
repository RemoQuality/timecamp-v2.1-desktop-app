#include "UIAControlIterator.h"

bool UIAControlIterator::informError = false;

std::wstring UIAControlItem::getDescription()
{
	std::wstring value = L"";
	VARIANT var;
	VariantInit( &var );
	HRESULT r = node->GetCurrentPropertyValue(UIA_LegacyIAccessibleDescriptionPropertyId,&var);
	
	if (r == S_OK  && var.vt != VT_EMPTY && var.bstrVal) {
		value = (LPCWSTR)var.bstrVal;

	}
	else
	{
		value = L"";
	}
	VariantClear(&var);

	return value;
}

std::wstring UIAControlItem::getRoleText()
{
	std::wstring value = L"";
	return value;
}

long UIAControlItem::getRole()
{
	long role = 0;
	return role;
}

std::wstring UIAControlItem::getName()
{
	std::wstring value = L"";
	VARIANT var;
	VariantInit( &var );
	var.vt = VT_I4;
	var.lVal = 0;
	HRESULT r;
	BSTR bStrValue;

	r = node->get_CurrentName( &bStrValue );

	if (r == S_OK && var.vt != VT_EMPTY && bStrValue)
	{
		value = ( LPCWSTR )bStrValue;
	}
	else
	{
		value = L"";
	}
	SysFreeString(bStrValue);
	VariantClear(&var);

	if(value != L"")
	{
		r = node->GetCurrentPropertyValue(UIA_LegacyIAccessibleNamePropertyId, &var);
		if (r == S_OK && var.vt != VT_EMPTY && var.bstrVal)
			value = std::wstring(var.bstrVal, SysStringLen(var.bstrVal));
	}

	return value;
}

std::wstring UIAControlItem::getValue()
{
	VARIANT var;
	HRESULT r;
	std::wstring ws(L"");

	r = node->GetCurrentPropertyValue(UIA_ValueValuePropertyId,&var);
	if (r == S_OK && var.vt != VT_EMPTY && var.bstrVal)
		ws = std::wstring(var.bstrVal, SysStringLen(var.bstrVal));
	
	if(ws == L"")
	{
		r = node->GetCurrentPropertyValue(UIA_LegacyIAccessibleValuePropertyId,&var);
		if (r == S_OK && var.vt != VT_EMPTY && var.bstrVal)
			ws = std::wstring(var.bstrVal, SysStringLen(var.bstrVal));
	}
		
	return ws;
}