#ifdef GL_ES
  precision highp float;
#endif

  uniform vec3 mDiff;
  uniform vec3 mColor;
  uniform vec3 mAmb;

  varying vec3 vNormal;

  varying vec2 vTextureCoord;

#if alphaDepth
  uniform vec3 depthInfo;
  float ConvertDepth3(float d) { return (depthInfo.x*depthInfo.y)/(depthInfo.y-d*(depthInfo.y-depthInfo.x));  }
  // transform range in world-z to 0-1 for near-far
  float DepthRange( float d ) { return ( d - depthInfo.x ) / ( depthInfo.y - depthInfo.x ); }
#endif

#if hasColorMap
	uniform sampler2D colorMap;
#endif

#if hasBumpMap
	varying vec3 eyeVec; 
	uniform sampler2D bumpMap;
#endif


#if hasEnvSphereMap
	uniform sampler2D envSphereMap;
	uniform float envAmount;
#if hasNormalMap
 	varying vec3 u;
#else
	varying vec2 vEnvTextureCoord;
#endif
#endif

#if hasReflectMap
  uniform sampler2D reflectMap;
#endif

#if hasNormalMap
	uniform sampler2D normalMap;
#endif

	uniform float mAlpha;

#if hasAmbientMap
	uniform sampler2D ambientMap;
#endif

#if hasSpecularMap
	uniform sampler2D specularMap;
#endif

#if hasAlphaMap
	uniform sampler2D alphaMap;
#endif

#if lightPoint||lightDirectional||lightSpot||lightArea
  struct Light {
    vec3 lDir;
    vec3 lPos;
    vec3 lSpec;
    vec3 lDiff;
    float lInt;
    float lDist;
  };
  uniform Light lights[loopCount];  
	varying vec3 lightDir[loopCount];
#endif

uniform vec3 mSpec;
uniform float mShine;
uniform vec3 lAmb;

#if lightPoint||lightSpot
  varying vec3 lightPos[loopCount];
#endif



varying vec3 camPos;
varying vec4 vPosition;

uniform mat4 uPMatrix;

void main(void) 
{
	vec3 n;
	vec4 color = vec4(0.0,0.0,0.0,0.0);
	
#if hasBumpMap
  float height = texture2D(bumpMap, vTextureCoord.xy).r;  
  float v = (height) * 0.05 - 0.04; // * scale and - bias 
  vec3 eye = normalize(eyeVec); 
  vec2 texCoord = vTextureCoord.xy + (eye.xy * v);
#else 
#if hasColorMap||hasBumpMap||hasNormalMap||hasAmbientMap||hasSpecularMap||hasAlphaMap
	vec2 texCoord = vTextureCoord;
#endif
#endif


#if hasNormalMap
 		vec3 bumpNorm = vec3(texture2D(normalMap, texCoord));

		n = (vec4(normalize(vNormal),1.0)).xyz;
    bumpNorm = (bumpNorm-0.5)*2.0;
    bumpNorm.y = -bumpNorm.y;
    n = normalize((n+bumpNorm)/2.0);
#else
		n = normalize(vNormal);
#endif


#if hasColorMap
#if !(lightPoint||lightDirectional||lightSpot||lightArea)
	color = texture2D(colorMap, vec2(texCoord.s, texCoord.t)).rgba;
	//vec4(lAmb,1.0)*
#else
  color = texture2D(colorMap, vec2(texCoord.s, texCoord.t)).rgba;
  color.rgb *= mColor;
#endif
	if (color.a<=0.9) discard;  
#else
	color = vec4(mColor,1.0);
#endif

#if hasAlphaMap
	color.a = texture2D(alphaMap, texCoord).r;
  if (color.a==0.0) discard;
#else
#if hasAlpha
	color.a = mAlpha;
#endif
#endif


//float envAmount = 1.0;

vec3 accum = lAmb;


#if lightPoint
  float dist;
	float NdotL;

	float NdotHV = 0.0;
  float att = 0.0;

  vec3 halfVector;
  vec3 specTotal = vec3(0.0,0.0,0.0);

  for (int i = 0; i < loopCount; i++) {
  
	  halfVector = normalize(vec3(0.0,0.0,1.0)+lightDir[i]);

    dist = length(lightPos[i]-vPosition.xyz);

  	NdotL = max(dot(normalize(lightDir[i]),n),0.0);

  	if (NdotL > 0.0) {
  		// basic diffuse
      att = clamp(((lights[i].lDist-dist)/lights[i].lDist), 0.0, 1.0)*lights[i].lInt;

  		accum += att * NdotL * lights[i].lDiff * mDiff;

   		NdotHV = max(dot(n, halfVector),0.0);

	
	    #if hasSpecularMap
			  vec3 spec2 = lights[i].lSpec * texture2D(specularMap, vec2(texCoord.s, texCoord.t)).rgb * pow(NdotHV,mShine);
	    #else
			  vec3 spec2 = lights[i].lSpec * mSpec * pow(NdotHV,mShine);
	    #endif
  
        specTotal += spec2;
  	}
  	
  }
  
  color.rgb *= accum;
  color.rgb += specTotal;
#endif




#if lightDirectional
  float NdotL;
  float NdotHV = 0.0;
  vec3 specTotal = vec3(0.0,0.0,0.0);
  vec3 spec2 = vec3(0.0,0.0,0.0);

	vec3 halfVector;
  
  for (int i = 0; i < loopCount; i++) {

	  halfVector = normalize(vec3(0.0,0.0,1.0)-lightDir[i]);

  	NdotL = max(dot(normalize(-lightDir[i]),n),0.0);

  	if (NdotL > 0.0) 	{
  		accum += lights[i].lInt * mDiff * lights[i].lDiff * NdotL;		

   		NdotHV = max(dot(n, halfVector),0.0);

      #if hasSpecularMap
        spec2 = lights[i].lSpec * texture2D(specularMap, vec2(texCoord.s, texCoord.t)).rgb * pow(NdotHV,mShine);
      #else
        spec2 = lights[i].lSpec * mSpec * pow(NdotHV,mShine);
      #endif
      
      specTotal += spec2;
  	}
  }  
  
  color.rgb *= accum;
  color.rgb += specTotal;
#endif


#if hasReflectMap
  float environmentAmount = texture2D( reflectMap, texCoord).r;
#endif

#if hasEnvSphereMap
#if hasNormalMap
	vec3 r = reflect( u, n );
	float m = 2.0 * sqrt( r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0) );

	vec3 coord;
	coord.s = r.x/m + 0.5;
	coord.t = r.y/m + 0.5;
	
  #if hasReflectMap
		color.rgb += mColor*accum*texture2D( envSphereMap, coord.st).rgb * environmentAmount;
	 #else
		color.rgb += mColor*accum*texture2D( envSphereMap, coord.st).rgb * envAmount;
	 #endif

#else
	#if hasReflectMap
 	  color.rgb += mColor*accum*texture2D( envSphereMap, vEnvTextureCoord).rgb * environmentAmount;
	#else
	 	color.rgb += mColor*accum*texture2D( envSphereMap, vEnvTextureCoord).rgb*envAmount;
	#endif
#endif

#endif



#if hasAmbientMap
#if lightPoint||lightDirectional||lightSpot||lightArea
  color.rgb += texture2D(ambientMap, texCoord).rgb*(vec3(1.0,1.0,1.0)+mColor*mAmb);
#else
  color.rgb = color.rgb*texture2D(ambientMap, texCoord).rgb;							
#endif
#else
#if !hasColorMap
	color.rgb += mColor*mAmb;
#else
  color.rgb += mAmb*texture2D(colorMap, texCoord).rgb;
#endif
#endif


#if alphaDepth
#if !hasAlpha
  float linear_depth = DepthRange( ConvertDepth3( gl_FragCoord.z ));

  color.a = linear_depth;
#endif
#endif

	gl_FragColor = clamp(color,0.0,1.0);
}