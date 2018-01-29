#include "AccControlIterator.h"


std::wstring AccControlItem::getDescription()
{
	std::wstring value = L"";
	HRESULT r;
	VARIANT var;
	VariantInit( &var );
	var.vt = VT_I4;
	var.lVal = 0;
	BSTR bStrValue = 0;
	r = node->get_accDescription(var, &bStrValue);
	if (r == S_OK && var.vt != VT_EMPTY && bStrValue) {
		value = ( LPCWSTR )bStrValue;

	} else {
		value = L"";
	}
	SysFreeString(bStrValue);
	VariantClear(&var);

	return value;
}

std::wstring AccControlItem::getRoleText()
{
	std::wstring value = L"";
	long role = this->getRole();
	int roleLength = 100;
	LPTSTR bStrValue = (LPTSTR)malloc((roleLength+1) * sizeof(TCHAR));

	GetRoleText(role, bStrValue, 100);
	if( bStrValue ) {
		value = bStrValue;
	} else {
		value = L"";
	}

	free(bStrValue);

	return value;
}

long AccControlItem::getRole()
{
	HRESULT r;
	long role = 0;
	VARIANT var, v1;
	VariantInit( &var );
	VariantInit( &v1 );
	var.vt = VT_I4;
	var.lVal = 0;

	v1.vt = VT_I4;
	r = node->get_accRole(var, &v1);
	if (r == S_OK && v1.vt != VT_EMPTY)
	{
		role = v1.llVal;
	}
	VariantClear(&v1);
	VariantClear(&var);
	return role;
}

std::wstring AccControlItem::getName()
{
	std::wstring value = L"";
	VARIANT var;
	VariantInit( &var );
	var.vt = VT_I4;
	var.lVal = 0;
	BSTR bStrValue = 0;
	HRESULT r;
	r = node->get_accName(var, &bStrValue);
	if (r == S_OK && var.vt != VT_EMPTY && bStrValue) {
		value = ( LPCWSTR )bStrValue;

	} else {
		value = L"";
	}
	SysFreeString(bStrValue);
	VariantClear(&var);

	return value;
}

std::wstring AccControlItem::getValue()
{
	std::wstring value = L"";
	VARIANT var;
	VariantInit( &var );
	var.vt = VT_I4;
	var.lVal = 0;
	BSTR bStrValue = 0;
	HRESULT r = node->get_accValue(var, &bStrValue);
	if (r == S_OK && var.vt != VT_EMPTY && bStrValue)
	{
		value = ( LPCWSTR )bStrValue;
	}
	else 
	{
		value = L"";
	}
	if( value != L"" ) {

	}
	SysFreeString(bStrValue);
	VariantClear(&var);

	return value;
}
