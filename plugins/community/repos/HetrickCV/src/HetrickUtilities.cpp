
float LERP(const float _amountOfA, const float _inA, const float _inB)
{
    return ((_amountOfA*_inA)+((1.0f-_amountOfA)*_inB));
}