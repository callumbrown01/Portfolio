varying vec4 position;
varying vec3 normal;
vec4 color;
varying vec2 texCoord;  // The third coordinate is always 0.0 and is discarded

uniform sampler2D texture;
uniform float texScale;

uniform vec3 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform float Shininess;

// light 1
uniform vec4 LightPosition;

// light 2 'sun'
uniform vec4 LightPosition2;
float LightBrightness2 = 0.0;

// light 3 'spotlight'
uniform vec4 LightPosition3;
//uniform vec3 LightDir;
vec3 LightDir = vec3(0.1, -4.0, 0.1);
float cutOff = 0.997;
vec3 diffuse3;

// PART F: values for light attenuation equation
float constant = 1.0;
float linear = 0.09;
float quadratic = 0.032;

void main()
{
    // use fragment position instead of vertex position as in vertex shader
    vec3 pos = (ModelView * position).xyz;
    
    // distance between light source and the fragment
    vec3 Lvec = LightPosition.xyz - pos;
    vec3 Lvec2 = LightPosition2.xyz - pos;
    vec3 Lvec3 = LightPosition3.xyz - pos;

    // unit vectors for shading calculations
    vec3 L = normalize(Lvec);   // Direction to the light source
    vec3 L2 = normalize(Lvec2); // Direction to light 2
    vec3 L3 = normalize(Lvec3); // Direction to spotlight
    vec3 E = normalize(-pos);   // Direction to the eye/camera
    vec3 H = normalize(L + E);  // Halfway vector
    vec3 H2 = normalize(L2 + E); // Halfway vector for light 2
    vec3 H3 = normalize(L3 + E); // Halfway vector for spotlight

    // transform fragment normal to eye coordinates (assuming uniform scaling)
    vec3 N = normalize((ModelView * vec4(normal, 0.0)).xyz);

    // TASK J: spotlight calculations
    vec3 lightDir = normalize(Lvec3);
    float theta = dot(LightDir, lightDir);

    if (theta > cutOff && LightPosition3.y > 0.0) {
        
        float Kd3 = max(dot(lightDir, N), 0.0);
        diffuse3 = Kd3 * DiffuseProduct;
    }

    // TASK I: adjust brightness of light 2 based on y value (height);
    float sunFactor = LightPosition2.y/3.5;
    LightBrightness2 += sunFactor;

    // compute terms in the illumination equation
    vec3 ambient = AmbientProduct;

    float Kd = max(dot(L, N), 0.0);
    vec3 diffuse = Kd * DiffuseProduct;
    float Kd2 = max(dot(L2, N), 0.0);
    vec3 diffuse2 = Kd2 * DiffuseProduct * LightBrightness2;

    float Ks = pow(max(dot(N, H), 0.0), Shininess);
    vec3 specular = Ks * SpecularProduct;
    float Ks2 = pow(max(dot(N, H2), 0.0), Shininess);
    vec3 specular2 = Ks2 * SpecularProduct * LightBrightness2;

    if (dot(L, N) < 0.0) {
        specular = vec3(0.0, 0.0, 0.0);
    }

    if (dot(L2, N) < 0.0) {
        specular2 = vec3(0.0, 0.0, 0.0);
    }

    vec3 globalAmbient = vec3(0.2, 0.2, 0.2);

    // PART F: Light Attenuation as in vStart.glsl
    // float attenuationFactor = 1.0 / (constant + linear * length(Lvec)) + 
    //                           quadratic * (length(Lvec) * length(Lvec));

    // ambient *= attenuationFactor;
    // diffuse *= attenuationFactor;
    // specular *= attenuationFactor;

    float lightDropoff = 0.008 + length(Lvec);

    ambient /= lightDropoff;
    diffuse /= lightDropoff;
    specular /= lightDropoff;

    color.rgb = globalAmbient + ambient + diffuse + diffuse2 + diffuse3;
    color.a = 1.0;
    
    // PART H: seperating specular lighting component from the colour aspect of the light source.
    gl_FragColor = (color * texture2D( texture, texCoord * 2.0 * texScale)) + vec4(specular + specular2, 1.0);

    // adding some light protection for when the 'sun' is below the 'horizon' (the light has -y coordinates)
    if (LightPosition2.y < 0.0) {
        color.rgb = globalAmbient + ((ambient + diffuse) / lightDropoff);
        gl_FragColor = (color * texture2D(texture, texCoord * 2.0 * texScale)) + vec4(specular, 1.0);
    }
}

