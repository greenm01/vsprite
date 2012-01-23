
A 2D OpenGL vector sprite animation engine, extracted from OpenMelee

Loads and draws shapes from SVG files (created in Inkscape, for example).
Designed to be simple and efficient, not robust or complete.
It supports typically used SVG shapes, including curves and paths, with gradient color fills.
It should 'play nice' with 2D physics engines like Chipmunk and Box2D.

At some point, we may add support for other vector graphics formats, as SVG has a few shortcomings and is slow to evolve.


## Dependencies:

- Poly2tri-C ... no, not needed
- OpenGL libs...


## Intended Use:

Vsprite is pretty small, and you may wish to modify it, so I suggest that you incorporate it into your application's source tree and build process, rather than treat it as a separately packaged library.

There are bigger, more robust libraries that do the same thing; what I like about Vsprite is, it's simple and direct, with no external dependencies except for OpenGL....


## Status:

May 2011 -- just started porting to plain C and decoupling from OpenMelee,
Poly2tri, Box2D.




