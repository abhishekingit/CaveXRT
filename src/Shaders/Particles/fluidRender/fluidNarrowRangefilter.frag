#version 430 core

in vec2 vUV;
out float FilteredDepth;

uniform sampler2D inputDepthTexture;
uniform sampler2D fluidThicknessTexture;

uniform vec2 texelSize;
uniform vec2 direction;

uniform int filterRadius;
uniform float sigmaSpatial;
uniform float sigmaRangeScale;
uniform float sigmaRangeBase;
uniform float thicknessEpsilon;

uniform float particleRadius;
uniform float thresholdRatio;
uniform float clampRatio;
uniform int minFilterRadius;	

uniform float depthAdaptiveScale;
uniform float minSigmaSpatial;

const int HARD_MAX_RADIUS = 100;

float gaussianWeight(float r, float twoSigma2)
{
    return exp(-(r * r) / twoSigma2);
}

void modifiedGaussianFilter1D(
    inout float sampleDepth,
    inout float weight,
    inout float upper,
    inout float lower,
    float lowerClamp,
    float threshold
) {
    if (sampleDepth > upper) {
        weight = 0.0;
    } else {
        if (sampleDepth < lower) {
            sampleDepth = lowerClamp;
        } else {
            upper = max(upper, sampleDepth + threshold);
            lower = min(lower, sampleDepth - threshold);
        }
    }
}

void main() {
	float centerThickness = texture(fluidThicknessTexture, vUV).r;
	float centerDepth = texture(inputDepthTexture, vUV).r;

	if(centerThickness <= thicknessEpsilon || centerDepth <= 0.0) {
		FilteredDepth = 0.0;
		return;
	}

	float threshold = particleRadius * thresholdRatio;
	float upper = centerDepth + threshold;
	float lower	= centerDepth - threshold;
	float lowerClamp = centerDepth - particleRadius * clampRatio;

	float sigmaRange = max(1e-5, sigmaRangeBase + sigmaRangeScale * centerDepth);

	float depthFactor = 1.0 / (1.0 + depthAdaptiveScale * centerDepth);
	int maxRadius = min(filterRadius, HARD_MAX_RADIUS);
	int minRadius = clamp(minFilterRadius, 1, maxRadius);
	int radiusCap = clamp(int(round(float(maxRadius) * depthFactor)), minRadius, maxRadius);
	//int radiusCap = clamp(filterRadius, 1, min(minFilterRadius, HARD_MAX_RADIUS));
	
	//float sigmaS = max(0.35, sigmaSpatial);
	float sigmaS = max(minSigmaSpatial, sigmaSpatial * depthFactor);
	float twoSigmaSpatial2 = 2.0 * sigmaS * sigmaS;
	float twoSigmaRange2 = 2.0 * sigmaRange * sigmaRange;

	


	float wSum = 1.0;
	float dSum = centerDepth;

	for(int i = 1; i <= radiusCap; i++) {
		float r2 = float(i * i);
		float spatialW = gaussianWeight(float(i), twoSigmaSpatial2);

		vec2 offset = direction * texelSize * float(i);
		//positive side
		{
			float d = texture(inputDepthTexture, vUV + offset).r;
			float t = texture(fluidThicknessTexture, vUV + offset).r;

			if(t > thicknessEpsilon && d > 0.0) {
				float dd = d - centerDepth;
				if(abs(dd) <= 3.0 * sigmaRange) {
					float w = spatialW * gaussianWeight(dd, twoSigmaRange2);
					modifiedGaussianFilter1D(d, w, upper, lower, lowerClamp, threshold);
					dSum += d * w;
					wSum += w;

				}



			}




		}

		//negative side

		{
			float d = texture(inputDepthTexture, vUV - offset).r;
			float t = texture(fluidThicknessTexture, vUV - offset).r;

			if(t > thicknessEpsilon && d > 0.0) {
				float dd = d - centerDepth;
				if(abs(dd) <= 3.0 * sigmaRange) {
					float w = spatialW * gaussianWeight(dd, twoSigmaRange2);
					modifiedGaussianFilter1D(d, w, upper, lower, lowerClamp, threshold);
					dSum += d * w;
					wSum += w;

				}


			}

		}

	}
	
	FilteredDepth = (wSum > 1e-6) ? (dSum / wSum) : centerDepth;
}