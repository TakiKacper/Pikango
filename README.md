# Pikango

Pikango is just another RHI (Rendering Hardware Interface) I created for use in further projects. 

Library allows to:
- Build api-agnostic renderers
- Call to graphics api from any thread
- Build memory safe applications (pikango provide automatic reference-counting-based garbage collection)

# Should I use Pikango

Probably not.  
  
Pikango is heavily in development, hardly tested and it's api support is really narrow (OpenGL 4.3 only at this point).

As I continue working on other projects involving pikango, it will gradually get more refined, it's specification rigider, and support broader.

As if for today I recommend you taking a look at [bgfx](https://github.com/bkaradzic/bgfx)
