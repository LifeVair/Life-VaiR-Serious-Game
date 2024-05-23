float3 texColor = Texture2DSample(Tex,TexSampler, UV);

bool isWithinMargin = all(abs(texColor - TargetColor) <= MarginOfError);

if (isWithinMargin)
{
	return float3(1.0, 1.0, 1.0); // White
}
else
{
	return float3(0.0, 0.0, 0.0); // Black
}