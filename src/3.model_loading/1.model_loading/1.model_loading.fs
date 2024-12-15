   // 1.model_loading.fs
   #version 330 core
   out vec4 FragColor;

   in vec2 TexCoords;

   uniform sampler2D texture_diffuse1;
   uniform float time;
   uniform bool animateTexture;
   uniform vec2 movementDirection;

   void main()
   {
       vec2 animatedTexCoords = TexCoords;

       // Apply texture offset only if animation is enabled
       if (animateTexture)
       {
           animatedTexCoords += movementDirection * time;
       }

       FragColor = texture(texture_diffuse1, animatedTexCoords);
   }
   