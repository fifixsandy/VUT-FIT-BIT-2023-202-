/*!
 * @file
 * @brief This file contains functions for model rendering
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#include <student/prepareModel.hpp>
#include <student/gpu.hpp>

///\endcond

#include <cstdio>

void prepareNode(GPUMemory&mem,CommandBuffer&cb,Node const&node,Model const&model,glm::mat4 const&prubeznaMatice, uint32_t &drawCount)
{
  if(node.mesh>=0)
  {
    Mesh mesh                         = model.meshes[node.mesh];

    VertexArray vao;
    vao.indexBufferID                 = mesh.indexBufferID;
    vao.indexOffset                   = mesh.indexOffset;
    vao.indexType                     = mesh.indexType;

    vao.vertexAttrib[0]               = mesh.position;
    vao.vertexAttrib[1]               = mesh.normal;
    vao.vertexAttrib[2]               = mesh.texCoord;

    mem.vertexArrays[drawCount]       = vao;

    pushBindVertexArrayCommand(cb, drawCount);
    pushDrawCommand(cb, mesh.nofIndices, !mesh.doubleSided);

    mem.uniforms[10+drawCount*5+0].m4 = prubeznaMatice * node.modelMatrix;
    mem.uniforms[10+drawCount*5+1].m4 = glm::transpose(glm::inverse(prubeznaMatice * node.modelMatrix));
    mem.uniforms[10+drawCount*5+2].v4 = mesh.diffuseColor;
    mem.uniforms[10+drawCount*5+3].i1 = mesh.diffuseTexture;
    mem.uniforms[10+drawCount*5+4].v1 = mesh.doubleSided;
    
    drawCount++;
  } 
  for(size_t i=0;i<node.children.size();++i)
  {
    prepareNode(mem, cb, node.children[i], model, prubeznaMatice * node.modelMatrix, drawCount);
  }
}


/**
 * @brief This function prepares model into memory and creates command buffer
 *
 * @param mem gpu memory
 * @param commandBuffer command buffer
 * @param model model structure
 */
//! [drawModel]
void prepareModel(GPUMemory&mem,CommandBuffer&cb,Model const&model){
  /// \todo Tato funkce připraví command buffer pro model a nastaví správně pamět grafické karty.<br>
  /// Vaším úkolem je správně projít model a vložit vykreslovací příkazy do commandBufferu.
  /// Zároveň musíte vložit do paměti textury, buffery a uniformní proměnné, které buffer command buffer využívat.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace a v testech.
  for(uint32_t i = 0; i < model.buffers.size(); i++)
  {
    mem.buffers[i]  = model.buffers[i];
  }

  for(uint32_t j = 0; j < model.textures.size(); j++)
  {
    mem.textures[j] = model.textures[j];
  }
  

  uint32_t drawCount          = 0;
  glm::mat4 jednotkovaMatrice = glm::mat4(1.f);

  for(size_t i=0;i<model.roots.size();++i)
  {
    prepareNode(mem, cb, model.roots[i], model, jednotkovaMatrice, drawCount);
  }
}
//! [drawModel]

/**
 * @brief This function represents vertex shader of texture rendering method.
 *
 * @param outVertex output vertex
 * @param inVertex input vertex
 * @param si shader interface
 */
//! [drawModel_vs]
void drawModel_vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&si){
  auto gl_DrawID = si.gl_DrawID;
  
  outVertex.attributes[0].v3 = si.uniforms[10+gl_DrawID*5+0].m4 * glm::vec4(inVertex.attributes[0].v3, 1.f);
  outVertex.attributes[1].v3 = glm::transpose(glm::inverse(si.uniforms[10+gl_DrawID*5+0].m4)) * glm::vec4(inVertex.attributes[1].v3, 0.f);
  outVertex.attributes[2].v2 = inVertex.attributes[2].v2;
  outVertex.gl_Position = si.uniforms[0].m4 * si.uniforms[10+gl_DrawID*5+0].m4 * glm::vec4(inVertex.attributes[0].v3, 1.f);
  outVertex.attributes[3].v4 = si.uniforms[3].m4 * si.uniforms[10+gl_DrawID*5+0].m4 * glm::vec4(inVertex.attributes[0].v3, 1.f);

  /// \todo Tato funkce reprezentujte vertex shader.<br>
  /// Vaším úkolem je správně trasnformovat vrcholy modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawModel_vs]

#include<iostream>

/**
 * @brief This functionrepresents fragment shader of texture rendering method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param si shader interface
 */
//! [drawModel_fs]
void drawModel_fragmentShader(OutFragment&outFragment,InFragment const&inFragment,ShaderInterface const&si){
  
  auto gl_DrawID = si.gl_DrawID;
  glm::vec4 dC;
  glm::vec4 aL;
  glm::vec4 dL;
  auto aLC = glm::vec4(si.uniforms[7].v3, 1.0f);
  auto dLC = glm::vec4(si.uniforms[8].v3, 1.0f);
  auto N = glm::normalize(inFragment.attributes[1].v3);
  auto L = glm::normalize(si.uniforms[1].v3 - inFragment.attributes[0].v3);
  auto dF = glm::clamp(glm::dot(L,N), 0.f, 1.f);
  if(si.uniforms[10+gl_DrawID*5+3].i1 < 0)
  {
    dC  = si.uniforms[10+gl_DrawID*5+2].v4;
  }
  else
  {
    dC  = read_texture(si.textures[si.uniforms[10+gl_DrawID*5+3].i1], inFragment.attributes[2].v2);
  }

  aL = dC * aLC;
  dL = dC * dLC * dF;


  outFragment.gl_FragColor = aL + dL;
  outFragment.gl_FragColor[3] = dC.a;

  if(dC.a < 0.5)
  {
    outFragment.discard = true;
  }



  /// \todo Tato funkce reprezentujte fragment shader.<br>
  /// Vaším úkolem je správně obarvit fragmenty a osvětlit je pomocí lambertova osvětlovacího modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.

}
//! [drawModel_fs]

