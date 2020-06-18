#pragma once

class Register
{
//Instance Data
private:
	uint8_t value = 0x00;
public:

//Member Functions
private:
public:
	Register();
	Register(uint8_t val);
	void setValue(uint8_t newValue);
	uint8_t getValue();

};

Register::Register()
{
}
Register::Register(uint8_t val)
{
	this->value = val;
}
void Register::setValue(uint8_t newValue)
{
	this->value = newValue;
}
uint8_t Register::getValue()
{
	return this->value;
}