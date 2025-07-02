#version 100

precision highp float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables
uniform vec2 size;   // render size

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}


void main()
{
    // Texel color fetching from texture sampler
    vec4 source = texture2D(texture0, fragTexCoord);

    //const int range = int((samples - 1.0) / 2.0);

    //declare stuff
	const int mSize = 15;
	const int kSize = (mSize-1)/2;
	float kernel[mSize];
	vec3 sum = vec3(0.0);
		
	//create the 1-D kernel
	float sigma = 7.0;
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), sigma);
	}
		
	//get the normalization factor (as the gaussian has been clamped)
	for (int j = 0; j < mSize; ++j)
	{
		Z += kernel[j];
	}
		
	//read out the texels
	for (int i=-kSize; i <= kSize; ++i)
	{
		for (int j=-kSize; j <= kSize; ++j)
		{
			sum += kernel[kSize+j]*kernel[kSize+i]*texture2D(texture0, (fragTexCoord.xy + vec2(float(i),float(j)) / size) ).rgb;
		}
	}

    const float gamma = 1.0;
    const float exposure = 2.0;

    vec3 hdrColor = source.rgb;      
    vec3 bloomColor = sum.rgb / (Z * Z);

    vec3 bloomed = (hdrColor + bloomColor);// additive blending

    // tone mapping
    vec3 result = vec3(1.0) - exp(-bloomed * exposure);

    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));

    // Calculate final fragment color
    gl_FragColor = vec4(result, 1.0);
}
