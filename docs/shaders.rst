==================
SensEngine Shaders
==================

.. contents::

The Engine
==========

SensEngine supports a powerful material system, allowing custom
shaders at any stage in the rendering pipeline. This first section
touches on the facilities that SensEngine provides to shader writers.

.. Render Targets:

Render Targets
--------------

SensEngine can provide RGBA8, RG16F, and RGBA16F render
targets. Defining new render targets is outside the scope of this
guide, and currently requires modification of the SensEngine source
code.

The existing render targets are listed below. These names will be used
throughout this documentation. You must use these names when defining
fragment shader outputs, but you can rename them through the material
uniform binding system when using them as textures.


* ``gcol``
    GBuffer color value. RGBA8 buffer. RGB are the color value. Alpha
    value can only be used for alpha-test antialiasing, not for
    blending.

* ``gnor``
    GBuffer normal values. RGBA8 buffer. RGB is the normal, which must
    be converted form the -1..1 range to 0..1. Alpha value is the
    specular blend factor (0..1)

* ``gmat``
    GBuffer material properties. RG16F buffer. R is the specular
    exponent. G is surface emission.

* ``lcol``
    Lighting color buffer. HDR RGBA16F buffer. This is the only buffer
    that can be ping-ponged (read and written in the same pass)

.. Render Passes:

Render Passes
-------------

SensEngine draws geometry (and thus shaders) in a number of
passes. These passes define the types of render targets that are drawn
to, the information available to your shader about the scene, and the
overall completeness of the scene's rendering.

A shader must write sensible values to all render targets used in a
given pass. Failure to do so will cause undefined, but almost
certainly incorrect, behavior.

Geometry
~~~~~~~~

The **Geometry** pass is the "normal" rendering pass for
geometry. Most surface shaders will be written for this pass.

* Render Targets used in drawing:
    - gcol
    - gnor
    - gmat

* Render Targets available as textures:
    - none

Lighting
~~~~~~~~

The **Lighting** pass takes the data written by the geometry pass and
fills a color buffer with lit pixels. Custom geometry cannot be
specified for this pass.  Shaders in this pass are only applied to
lamps.

* Render Targets used in drawing:
    - lcol

* Render Targets available as textures:
    - gcol
    - gnor
    - gmat

Post-Lighting
~~~~~~~~~~~~~

The **Post-Lighting** pass is for special geometry that must be drawn
after the main world has been lit. Exapmles include glass and water.

* Render Targets used in drawing:
    - lcol

* Render Targets available as textures:
    - gcol
    - gnor
    - gmat
    - lcol (requires blit)

Pre-Tone Effect
~~~~~~~~~~~~~~~

Shaders in the **Pre-Tone Effect** pass are drawn fullscreen. They
have access to the full HDR range of the lighting buffer. Custom
geometry cannot be specified for this pass.

* Render Targets used in drawing:
    - lcol

* Render Targets available as textures:
    - gcol
    - gnor
    - gmat
    - lcol (ping-ponged)

Post-Tone Effect
~~~~~~~~~~~~~~~~

Shaders in the **Post-Tone Effect** pass are drawn fullscreen, after
tonemapping has been applied. They do not have access to the full HDR
lighting buffer, only the clamped values. Custom geometry cannot be
specified for this pass.

* Render Targets used in drawing:
    - lcol

* Render Targets available as textures:
    - gcol
    - gnor
    - gmat
    - lcol (ping-ponged)

Post-Effect
~~~~~~~~~~~

**Post-Effect** is the final rendering pass. Any geometry drawn in
this pass will bypass the entire pipeline and be drawn directly to the
screen. This is useful for HUDs and other game displays.

* Render Targets used in drawing:
    - lit

* Render Targets available as textures:
    - col
    - nor
    - mat
    - lit (ping-ponged)

The Shaders
===========

SensEngine shaders are written in GLSL version 1.50. You **must not**
use a GLSL ``#version`` directive - SensEngine will add this for
you. In addition, SensEngine provides the following preprocessor
definitions for use in your shaders:

  ``SENSE_MAX_INSTANCES``
     The maximum number of geometry instances that SensEngine will
     draw at one time. Use this for properly sizing uniform arrays for
     instances.

  ``SENSE_MAX_VTX_BONES``
     The maximum number of bones that can be used in any given skinned
     mesh

There is no defined standard for communication between shader
stages. If you wish to replace only a single shader stage, you should
read the existing shaders you plan to work with and follow the
conventions used.

Shader Inputs
-------------

* ``pos``
    Vertex position ``vec3``

* ``nor``
    Vertex normal ``vec3``

* ``tan``
    Vertex tangent ``vec3``

* ``col``
    Vertex color ``vec3``

* ``te0``
    Primary vertex texture coordinate ``vec2``

* ``te1``
    Secondary vertex texture coordinate ``vec2``

* ``ski``
    Skinning indices ``ivec4``

* ``skw``
    Skinning weights ``vec4``

Shader Outputs
--------------

For information on fragment shader outputs, see `Render Targets`_
and `Render Passes`_

Considerations
--------------

There are some special considerations when writing SensEngine shaders

* ModelView matrix inputs must be defined as an array of size
  ``SENSE_MAX_INSTANCES`` in order for hardware instancing to work
  correctly.

* Skinning matrix inputs must be defined as an array of size 
  ``SENSE_MAX_VTX_BONES``.

* If you intend for a shader to be used with vertex skinning, you must
  write a version of that shader specifically for that purpose. If
  using skinning, you must not define a normal ModelView matrix
  input. That matrix is premultiplied into the skinning bones.

* Skinning and instancing cannot be used together
