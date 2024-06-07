/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <student/gpu.hpp>

#include <cstdio>



bool setValue(Image *img, glm::vec4 val, uint32_t x, uint32_t y, bool depthTest, bool blend)
{
  if(img->data == nullptr) return false;
  
  uint8_t *uInt8Data = (uint8_t*)(img->data);
  uInt8Data += y*img->pitch + x*img->bytesPerPixel;
  if(img->format == Image::UINT8)
  {
    for(uint32_t i = 0; i < img->channels; ++i)
    {
      if(depthTest && uInt8Data[i] < val[1])
      {
        return true;
      }
      
      if(blend){uInt8Data[i] = (uint8_t)(((uInt8Data[i]/255.f)*(1.0f - val.w) + val[img->channelTypes[i]]*(val.w))*255.f);}
      else{uInt8Data[i] = (uint8_t)(((uInt8Data[i]/255.f)*(0.f) + val[img->channelTypes[i]]*(1.f))*255.f);}
    }
  }
  if(img->format == Image::FLOAT32)
  {
    float *floatData = (float*)uInt8Data;
    for(uint32_t i = 0; i < img->channels; ++i)
    {
      if(depthTest && floatData[i] < val[1])
      {
        return true;
      }
      if(blend){floatData[i] = floatData[i]*(1.0f - val.w) + val[img->channelTypes[i]]*(val.w);}
      else{floatData[i]      = floatData[i]*(0.f) + val[img->channelTypes[i]]*(1.f);}
    }
  }
  return false;
}

void clearImage(Image *img, uint32_t width, uint32_t height, glm::vec4 val, bool blend)
{
  for(uint32_t y = 0; y < height; ++y)
  {
    for(uint32_t x = 0; x < width; ++x)
    {
      setValue(img, val, x, y, false, blend);
    }
  }
}


void clear(Framebuffer* fbo, ClearCommand clearCommand)
{
  if(clearCommand.clearColor && fbo->color.data)
  {
    clearImage(&fbo->color, fbo->width, fbo->height, clearCommand.color, false);
  }
  if(clearCommand.clearDepth && fbo->depth.data)
  {
    clearImage(&fbo->depth, fbo->width, fbo->height, glm::vec4(clearCommand.depth), false);
  }
}

uint32_t computeVertexID(GPUMemory&mem,VertexArray const&vao,uint32_t shaderInvocation)
{
  if(vao.indexBufferID>=0)
  {
    const void *indexBuffer = mem.buffers[vao.indexBufferID].data;
    switch(vao.indexType)
    {
      case IndexType::UINT32:
      {
        uint32_t*ind   = (uint32_t*)((uint8_t*)indexBuffer+vao.indexOffset);
        return ind[shaderInvocation];
      }
      case IndexType::UINT16:
      {
        uint16_t*ind   = (uint16_t*)((uint8_t*)indexBuffer+vao.indexOffset);
        return ind[shaderInvocation];
      }
      case IndexType::UINT8:
      {
        uint8_t*ind    = ((uint8_t*)indexBuffer+vao.indexOffset);
        return ind[shaderInvocation];
      }
    }
  }

  return shaderInvocation;
}


void vertexAssemblyAtt(GPUMemory&mem, VertexArray const&vao, InVertex &InVertex)
{
  auto gl_VertexID = InVertex.gl_VertexID;
  for(uint32_t attributeNum = 0; attributeNum < maxAttributes; ++attributeNum)
  {
    if (vao.vertexAttrib[attributeNum].type == AttributeType::EMPTY) continue;
    auto *buff = mem.buffers[vao.vertexAttrib[attributeNum].bufferID].data;
    auto data = ((uint8_t*)buff+vao.vertexAttrib[attributeNum].offset+vao.vertexAttrib[attributeNum].stride*gl_VertexID);
    switch(vao.vertexAttrib[attributeNum].type)
    { 
      case AttributeType::FLOAT:
      
        InVertex.attributes[attributeNum].v1 = *(float*)data;
        break;
      
      case AttributeType::VEC2:
      
        InVertex.attributes[attributeNum].v2 = *(glm::vec2*)data;
        break;
      
      case AttributeType::VEC3:
      
        InVertex.attributes[attributeNum].v3 = *(glm::vec3*)data;
        break;
      
      case AttributeType::VEC4:
      
        InVertex.attributes[attributeNum].v4 = *(glm::vec4*)data;
        break;
      
      case AttributeType::UINT:
      
        InVertex.attributes[attributeNum].u1 = *(uint32_t*)data;
        break;
      
      case AttributeType::UVEC2:
      
        InVertex.attributes[attributeNum].u2 = *(glm::uvec2*)data;
        break;
      
      case AttributeType::UVEC3:
      
        InVertex.attributes[attributeNum].u3 = *(glm::uvec3*)data;
        break;
      
      case AttributeType::UVEC4:
      
        InVertex.attributes[attributeNum].u4 = *(glm::uvec4*)data;
        break;
    }
  }
  return;
}

struct triangle
{
  OutVertex points[3];
};

void vertexAssembly(InVertex &inV, VertexArray const&vao, uint32_t id, GPUMemory&mem)
{
  auto gl_VertexID = computeVertexID(mem, vao, id);
  inV.gl_VertexID  = gl_VertexID;
  vertexAssemblyAtt(mem, vao, inV);
}

void primitiveAssembly(triangle &t, Program const&prg, VertexArray const&vao, uint32_t tID, GPUMemory&mem)
{
  for(uint32_t i = 0; i < 3; ++i)
  {
      InVertex inV;
      ShaderInterface si;
      si.gl_DrawID = mem.gl_DrawID;
      si.uniforms  = mem.uniforms;
      si.textures  = mem.textures;
      vertexAssembly(inV, vao, tID+i, mem);
      prg.vertexShader(t.points[i], inV, si);
  }
}

void perspectiveDiv(triangle &t)
{
  for(uint32_t i = 0; i < 3; ++i)
  {
    t.points[i].gl_Position.x = t.points[i].gl_Position.x/t.points[i].gl_Position.w;
    t.points[i].gl_Position.y = t.points[i].gl_Position.y/t.points[i].gl_Position.w;
    t.points[i].gl_Position.z = t.points[i].gl_Position.z/t.points[i].gl_Position.w;
  }
}

void viewPointTransf(triangle &t, uint32_t w, uint32_t h)
{
  for(uint32_t i = 0; i < 3; ++i)
  {
    t.points[i].gl_Position.x = ((t.points[i].gl_Position.x) * 0.5 + 0.5) * (double)w;
    t.points[i].gl_Position.y = ((t.points[i].gl_Position.y) * 0.5 + 0.5) * (double)h;
  }
}


void inTriangle(float *E1, float *E2, float *E3, float ax, float bx, float cx, float ay, float by, float cy, bool xORy, float diff)
{
  if(xORy)
  {
    *E1 += ax;
    *E2 += bx;
    *E3 += cx;
  }
  else
  {
    *E1 = *E1 - diff*(ax) + ay;
    *E2 = *E2 - diff*(bx) + by; 
    *E3 = *E3 - diff*(cx) + cy;
  }
}



double triangleS(triangle &t)
{
  double p0x = t.points[0].gl_Position.x;
  double p0y = t.points[0].gl_Position.y;

  double p1x = t.points[1].gl_Position.x;
  double p1y = t.points[1].gl_Position.y;

  double p2x = t.points[2].gl_Position.x;
  double p2y = t.points[2].gl_Position.y;

  return 0.5f*(p0x*(p1y-p2y) + p1x*(p2y-p0y) + p2x*(p0y-p1y));
}

glm::vec3 calculateL(triangle&t, double x, double y, double S012)
{
  double p0x = t.points[0].gl_Position.x;
  double p0y = t.points[0].gl_Position.y;

  double p1x = t.points[1].gl_Position.x;
  double p1y = t.points[1].gl_Position.y;

  double p2x = t.points[2].gl_Position.x;
  double p2y = t.points[2].gl_Position.y;

  double S01P = 0.5f*(std::abs(p0x*(p1y-(double)y) + p1x*((double)y-p0y) + (double)x*(p0y-p1y)));
  double S0P2 = 0.5f*(std::abs(p0x*((double)y-p2y) + (double)x*(p2y-p0y) + p2x*(p0y-y)));
  double SP12 = 0.5f*(std::abs((double)x*(p1y-p2y) + p1x*(p2y-(double)y) + p2x*((double)y-p1y)));

  double L0 = SP12/S012;
  double L1 = S0P2/S012;
  double L2 = S01P/S012;

  return glm::vec3(L0, L1, L2);
}


double calculateZ(triangle &t, int32_t x, int32_t y, double S012)
{
  auto L = calculateL(t, x, y, S012);
  return (double)(L[0]*t.points[0].gl_Position.z + L[1]*t.points[1].gl_Position.z + L[2]*t.points[2].gl_Position.z);
}

void setInFrgZ(InFragment &inF, int32_t x, int32_t y, triangle &t, double S012)
{
  inF.gl_FragCoord.x = (double)x+0.5;
  inF.gl_FragCoord.y = (double)y+0.5;
  inF.gl_FragCoord.w = 1;
  inF.gl_FragCoord.z = calculateZ(t,x,y, S012);
}

void interpolation(triangle &t, int32_t x, int32_t y, AttributeType aT, InFragment &inF, double S012)
{
  auto L = calculateL(t, x+0.5, y+0.5, S012);
  double s = L[0]/t.points[0].gl_Position.w + L[1]/t.points[1].gl_Position.w + L[2]/t.points[2].gl_Position.w;
  double newL0 = L[0]/(s*t.points[0].gl_Position.w);
  double newL1 = L[1]/(s*t.points[1].gl_Position.w);
  double newL2 = L[2]/(s*t.points[2].gl_Position.w);
    if(aT != AttributeType::EMPTY)
    {
      if(aT == AttributeType::UINT)
      {
        inF.attributes->u1 = t.points[0].attributes->u1;
      }
      if(aT == AttributeType::FLOAT)
      {
        inF.attributes->v1 = t.points[0].attributes->v1*newL0 + t.points[1].attributes->v1*newL1 + t.points[2].attributes->v1*newL2;
      }
      if(aT == AttributeType::VEC2)
      {
        for(uint32_t i = 0; i < 2; ++i)
        {
          inF.attributes->v2[i] = t.points[0].attributes->v2[i]*newL0 + t.points[1].attributes->v2[i]*newL1 + t.points[2].attributes->v2[i]*newL2;
        }
      }
      if(aT == AttributeType::VEC3)
      {
        for(uint32_t i = 0; i < 3; ++i)
        {
          inF.attributes->v3[i] = t.points[0].attributes->v3[i]*newL0 + t.points[1].attributes->v3[i]*newL1 + t.points[2].attributes->v3[i]*newL2;
        }
      }
      if(aT == AttributeType::VEC4)
      {
        for(uint32_t i = 0; i < 4; ++i)
        {
          inF.attributes->v4[i] = t.points[0].attributes->v4[i]*newL0 + t.points[1].attributes->v4[i]*newL1 + t.points[2].attributes->v4[i]*newL2;
        }
      }
      if(aT == AttributeType::UVEC2)
      {
        inF.attributes->u2 = t.points[0].attributes->u2;
      }
      if(aT == AttributeType::UVEC3)
      {
        inF.attributes->u3 = t.points[0].attributes->u3;
      }
      if(aT == AttributeType::UVEC4)
      {
        inF.attributes->u4 = t.points[0].attributes->u4;
      }
    }
}


void setInFrg(InFragment &inF, int32_t x, int32_t y, triangle &t, Program const&prg, double S012)
{
  for(uint32_t i = 0; i < maxAttributes; ++i)
  {
    interpolation(t, x, y, prg.vs2fs[i], inF, S012);
  }
}


void swapCoordinates(triangle &t)
{
  auto tmp    = t.points[1];
  t.points[1] = t.points[2];
  t.points[2] = tmp;
}

float min(float x, float y)
{
  if(x<y)
  {
    return x;
  }
  return y;
}

float max(float x, float y)
{
  if(x>y)
  {
    return x;
  }
  return y;
}


void rasterize(uint32_t w, uint32_t h, triangle &t, Program const&prg, DrawCommand cc, GPUMemory&mem, Framebuffer &fb)
{ 
  auto A = t.points[0].gl_Position;
  auto B = t.points[1].gl_Position;
  auto C = t.points[2].gl_Position;

  auto ax = (A.y - B.y);
  auto bx = (B.y - C.y);
  auto cx = (C.y - A.y);
  
  auto ay = (B.x-A.x);
  auto by = (C.x-B.x);
  auto cy = (A.x-C.x);

  int32_t minX = max(0.f, min(A.x, min(B.x, C.x)));
  int32_t maxX = min((float)w, max(A.x, max(B.x, C.x)));
  int32_t minY = max(0.0f, min(A.y, min(B.y, C.y)));
  int32_t maxY = min((float)h, max(A.y, max(B.y, C.y)));

  auto diff = maxX - minX;

  float E1  = ax*(minX + 0.5) + ay*(minY + 0.5) - ax*A.x - ay*A.y;
  float E2  = bx*(minX + 0.5) + by*(minY + 0.5) - bx*B.x - by*B.y;
  float E3  = cx*(minX + 0.5) + cy*(minY + 0.5) - cx*C.x - cy*C.y;;
  double S  = std::abs(triangleS(t));

  for(int32_t y = minY; y < maxY; y++)
  { 
    for(int32_t x = minX; x < maxX; x++)
    {
      if(E1>=0 && E2>=0 && E3>=0)
      {
        InFragment inF;
        setInFrgZ(inF, (double)x, (double)y, t, S);
        setInFrg(inF, x, y, t, prg, S);

        OutFragment outF;
        ShaderInterface si;
        si.textures  = mem.textures;
        si.uniforms  = mem.uniforms;
        si.gl_DrawID = mem.gl_DrawID;

        prg.fragmentShader(outF, inF, si);
        if(!outF.discard)
        {
          uint32_t specialY = y;
          if(fb.yReversed)
          {
            specialY = h-y-1;
          }
          bool shouldSkip = setValue(&fb.depth, glm::vec4(inF.gl_FragCoord.z), x, specialY, true, false);
          if(!shouldSkip){setValue(&fb.color, outF.gl_FragColor, x, specialY, false, true);}
        }
      }
      
      E1 += ax;
      E2 += bx;
      E3 += cx;
    }
    E1 = E1 - diff*ax + ay;
    E2 = E2 - diff*bx + by;
    E3 = E3 - diff*cx + cy;
  }
} 

struct iLoveBees
{
  std::vector<int32_t> behind;
  std::vector<int32_t> front;

};

uint32_t behindCnt(triangle&t, iLoveBees&ilovebees)
{
  auto a       = t.points[0].gl_Position;
  auto b       = t.points[1].gl_Position;
  auto c       = t.points[2].gl_Position;

  bool aBehind = -a.w > a.z;
  bool bBehind = -b.w > b.z;
  bool cBehind = -c.w > c.z;


  if(!aBehind && !bBehind && !cBehind)
  {
    return 0;
  }
  if(aBehind && bBehind && cBehind)
  {
    return 3;
  }
  if(aBehind)
    ilovebees.behind.push_back(0);
  else
    ilovebees.front.push_back(0);
  if(bBehind)
    ilovebees.behind.push_back(1);
  else
    ilovebees.front.push_back(1);
  if(cBehind)
    ilovebees.behind.push_back(2);
  else
    ilovebees.front.push_back(2);

  return ilovebees.behind.size();
}

bool allBehind(triangle&t)
{
  iLoveBees ilb;
  if(behindCnt(t,ilb) == 3)
  {
    return true;
  }
  return false;
}


void intersect(OutVertex &firstV, OutVertex &secondV, OutVertex &newV, Program prg)
{
  auto firstP  = firstV.gl_Position;
  auto secondP = secondV.gl_Position;

  auto firstA  = firstV.attributes;
  auto secondA = secondV.attributes;

  float t = (-firstP.w - firstP.z) / (secondP.w - firstP.w + secondP.z - firstP.z);
    
  newV.gl_Position    = firstP + t * (secondP - firstP);

  for(uint32_t j = 0; j < maxAttributes; ++j)
  {
    switch(prg.vs2fs[j])
    {
      case AttributeType::EMPTY:
        continue;
      case AttributeType::UINT:
        newV.attributes[j].u1 = firstA[j].u1 + t*(secondA[j].u1 - firstA[j].u1);
        break;
      case AttributeType::FLOAT:
        newV.attributes[j].v1 = firstA[j].v1 + t*(secondA[j].v1 - firstA[j].v1);
        break;
      case AttributeType::UVEC2:
        for(uint32_t i = 0; i < 2; ++i)
        {
          newV.attributes[j].u2[i] = firstA[j].u2[i] + t*(secondA[j].u2[i] - firstA[j].u2[i]);
        }
        break;
      case AttributeType::UVEC3:
        for (uint32_t i = 0; i < 3; ++i) 
        {
          newV.attributes[j].u3[i] = firstA[j].u3[i] + t * (secondA[j].u3[i] - firstA[j].u3[i]);
        }
        break;
      case AttributeType::UVEC4:
        for (uint32_t i = 0; i < 4; ++i) 
        {
          newV.attributes[j].u4[i] = firstA[j].u4[i] + t * (secondA[j].u4[i] - firstA[j].u4[i]);
        }
        break;
      case AttributeType::VEC2:
        for (uint32_t i = 0; i < 2; ++i) 
        {
          newV.attributes[j].v2[i] = firstA[j].v2[i] + t * (secondA[j].v2[i] - firstA[j].v2[i]);
        }
        break;
      case AttributeType::VEC3:
        for (uint32_t i = 0; i < 3; ++i) 
        {
          newV.attributes[j].v3[i] = firstA[j].v3[i] + t * (secondA[j].v3[i] - firstA[j].v3[i]);
        }
        break;
      case AttributeType::VEC4:
        for (uint32_t i = 0; i < 4; ++i) 
        {
          newV.attributes[j].v4[i] = firstA[j].v4[i] + t * (secondA[j].v4[i] - firstA[j].v4[i]);
        }
        break;
    }
  }
}

struct clipped
{
  triangle tr[2];
  uint32_t cnt;
};

void clipping(triangle &Triangle, clipped &Clipped, iLoveBees &ilb, Program prg)
{
    uint32_t behindcnt = behindCnt(Triangle,ilb);
    if(behindcnt == 2)
    {
      intersect(Triangle.points[ilb.behind[0]], Triangle.points[ilb.front[0]], Clipped.tr[0].points[0], prg);
      intersect(Triangle.points[ilb.behind[1]], Triangle.points[ilb.front[0]], Clipped.tr[0].points[1], prg);
      Clipped.tr[0].points[2] = Triangle.points[ilb.front[0]];
      Clipped.cnt = 1;
    }
    if(behindcnt == 1)
    {
      intersect(Triangle.points[ilb.behind[0]], Triangle.points[ilb.front[0]], Clipped.tr[0].points[0], prg);
      intersect(Triangle.points[ilb.behind[0]], Triangle.points[ilb.front[1]], Clipped.tr[0].points[2], prg);
      Clipped.tr[0].points[1] = Triangle.points[ilb.front[0]];

      intersect(Triangle.points[ilb.behind[0]], Triangle.points[ilb.front[1]], Clipped.tr[1].points[0], prg);
      Clipped.tr[1].points[2] = Triangle.points[ilb.front[1]]; 
      Clipped.tr[1].points[1] = Triangle.points[ilb.front[0]];
      Clipped.cnt = 2;
    }
    if(behindcnt == 0)
    {
      Clipped.tr[0] = Triangle;
      Clipped.cnt = 1;
    }
}

void draw(GPUMemory&mem, CommandBuffer const&cb, DrawCommand cc)
{
  for(uint32_t i = 0; i < cc.nofVertices; i=i+3)
  {
    triangle Triangle;
    Program prg     = mem.programs[mem.activatedProgram];
    VertexArray vao = mem.vertexArrays[mem.activatedVertexArray];
    Framebuffer fb  = mem.framebuffers[mem.activatedFramebuffer];
    primitiveAssembly(Triangle, prg, vao, i, mem);
    if(triangleS(Triangle) == 0)
    { 
      continue;
    }
    if(triangleS(Triangle) < 0)
    {
      if(!cc.backfaceCulling)
      swapCoordinates(Triangle);
      else{continue;}
    }
    if(allBehind(Triangle)){continue;};
    iLoveBees ilb;
    clipped Clipped;
    clipping(Triangle, Clipped, ilb, prg);
    for(uint32_t j = 0; j < Clipped.cnt; ++j)
    {
      perspectiveDiv(Clipped.tr[j]);
      viewPointTransf(Clipped.tr[j], fb.width, fb.height);
      rasterize(fb.width, fb.height, Clipped.tr[j], prg, cc, mem, fb);
    }

  }
  mem.gl_DrawID++;
}



//! [izg_enqueue]
void izg_enqueue(GPUMemory&mem,CommandBuffer const&cb){

  /// \todo Tato funkce reprezentuje funkcionalitu grafické karty.<br>
  /// Měla by umět zpracovat command buffer, čistit framebuffer a kresli.<br>
  /// mem obsahuje paměť grafické karty.
  /// cb obsahuje command buffer pro zpracování.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
  mem.gl_DrawID  = 0;
  for(uint32_t i = 0;i<cb.nofCommands;++i)
  {
    if(cb.commands[i].type == CommandType::CLEAR)
      {
        ClearCommand cc  = cb.commands[i].data.clearCommand;
        Framebuffer *fbo = mem.framebuffers+mem.activatedFramebuffer;
        clear(fbo, cc);
      }
    if(cb.commands[i].type == CommandType::BIND_FRAMEBUFFER)
    {
      BindFramebufferCommand cc = cb.commands[i].data.bindFramebufferCommand;
      mem.activatedFramebuffer = cc.id;
    }
    if(cb.commands[i].type == CommandType::BIND_PROGRAM)
    {
      BindProgramCommand cc = cb.commands[i].data.bindProgramCommand;
      mem.activatedProgram  = cc.id;
    }
    if(cb.commands[i].type == CommandType::BIND_VERTEXARRAY)
    {
      BindVertexArrayCommand cc = cb.commands[i].data.bindVertexArrayCommand;
      mem.activatedVertexArray  = cc.id;
    }
    if(cb.commands[i].type == CommandType::DRAW)
    {
      DrawCommand cc = cb.commands[i].data.drawCommand;
      draw(mem, cb, cc);
    }
    if(cb.commands[i].type == CommandType::SET_DRAW_ID)
    {
      SetDrawIdCommand cc = cb.commands[i].data.setDrawIdCommand;
      mem.gl_DrawID = cc.id;
    }
    if(cb.commands[i].type == CommandType::SUB_COMMAND)
    {
      SubCommand cc = cb.commands[i].data.subCommand;
      izg_enqueue(mem, *cc.commandBuffer);
    }
  }

}
//! [izg_enqueue]

/**
 * @brief This function reads color from texture.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_texture(Texture const&texture,glm::vec2 uv){
  if(!texture.img.data)return glm::vec4(0.f);
  auto&img = texture.img;
  auto uv1 = glm::fract(glm::fract(uv)+1.f);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  return texelFetch(texture,pix);
}

/**
 * @brief This function reads color from texture with clamping on the borders.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_textureClamp(Texture const&texture,glm::vec2 uv){
  if(!texture.img.data)return glm::vec4(0.f);
  auto&img = texture.img;
  auto uv1 = glm::clamp(uv,0.f,1.f);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  return texelFetch(texture,pix);
}

/**
 * @brief This function fetches color from texture.
 *
 * @param texture texture
 * @param pix integer coorinates
 *
 * @return color 4 floats
 */
glm::vec4 texelFetch(Texture const&texture,glm::uvec2 pix){
  auto&img = texture.img;
  glm::vec4 color = glm::vec4(0.f,0.f,0.f,1.f);
  if(pix.x>=texture.width || pix.y >=texture.height)return color;
  if(img.format == Image::UINT8){
    auto colorPtr = (uint8_t*)getPixel(img,pix.x,pix.y);
    for(uint32_t c=0;c<img.channels;++c)
      color[c] = colorPtr[img.channelTypes[c]]/255.f;
  }
  if(texture.img.format == Image::FLOAT32){
    auto colorPtr = (float*)getPixel(img,pix.x,pix.y);
    for(uint32_t c=0;c<img.channels;++c)
      color[c] = colorPtr[img.channelTypes[c]];
  }
  return color;
}

