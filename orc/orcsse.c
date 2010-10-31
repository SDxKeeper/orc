
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcsse.h>
#include <orc/orcx86insn.h>

/**
 * SECTION:orcsse
 * @title: SSE
 * @short_description: code generation for SSE
 */


const char *
orc_x86_get_regname_sse(int i)
{
  static const char *x86_regs[] = {
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
  };

  if (i>=X86_XMM0 && i<X86_XMM0 + 16) return x86_regs[i - X86_XMM0];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}


#if 0
void
orc_sse_emit_pextrw_memoffset (OrcCompiler *p, int imm, int src,
    int offset, int dest)
{
  ORC_ASM_CODE(p,"  pextrw $%d, %%%s, %d(%%%s)\n", imm,
      orc_x86_get_regname_sse(src),
      offset, orc_x86_get_regname_ptr (p, dest));
  *p->codeptr++ = 0x66;
  orc_x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x3a;
  *p->codeptr++ = 0x15;
  orc_x86_emit_modrm_memoffset_old (p, src, offset, dest);
  *p->codeptr++ = imm;
}
#endif

void
orc_x86_emit_mov_memoffset_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      orc_sse_emit_movd_load_memoffset (compiler, offset, reg1, reg2);
      break;
    case 8:
      orc_sse_emit_movq_load_memoffset (compiler, offset, reg1, reg2);
      break;
    case 16:
      if (is_aligned) {
        orc_sse_emit_movdqa_load_memoffset (compiler, offset, reg1, reg2);
      } else {
        orc_sse_emit_movdqu_load_memoffset (compiler, offset, reg1, reg2);
      }
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_x86_emit_movhps_memoffset_sse (OrcCompiler *compiler, int offset,
    int reg1, int reg2)
{
  orc_sse_emit_movhps_load_memoffset (compiler, offset, reg1, reg2);
}

void
orc_x86_emit_mov_memindex_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %d(%%%s,%%%s,%d), %%%s\n", offset,
          orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_ptr(compiler, regindex), 1<<shift,
          orc_x86_get_regname_sse(reg2));
      *compiler->codeptr++ = 0x66;
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x6e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %d(%%%s,%%%s,%d), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_ptr(compiler, regindex), 1<<shift,
          orc_x86_get_regname_sse(reg2));
      *compiler->codeptr++ = 0xf3;
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7e;
      break;
    case 16:
      if (is_aligned) {
        ORC_ASM_CODE(compiler,"  movdqa %d(%%%s,%%%s,%d), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
            orc_x86_get_regname_ptr(compiler, regindex), 1<<shift,
            orc_x86_get_regname_sse(reg2));
        *compiler->codeptr++ = 0x66;
        orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
        *compiler->codeptr++ = 0x0f;
        *compiler->codeptr++ = 0x6f;
      } else {
        ORC_ASM_CODE(compiler,"  movdqu %d(%%%s,%%%s,%d), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
            orc_x86_get_regname_ptr(compiler, regindex), 1<<shift,
            orc_x86_get_regname_sse(reg2));
        *compiler->codeptr++ = 0xf3;
        orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
        *compiler->codeptr++ = 0x0f;
        *compiler->codeptr++ = 0x6f;
      }
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
  orc_x86_emit_modrm_memindex (compiler, reg2, offset, reg1, regindex, shift);
}

void
orc_x86_emit_mov_sse_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached)
{
  switch (size) {
    case 4:
      orc_sse_emit_movd_store_memoffset (compiler, offset, reg1, reg2);
      break;
    case 8:
      orc_sse_emit_movq_store_memoffset (compiler, offset, reg1, reg2);
      break;
    case 16:
      if (aligned) {
        if (uncached) {
          orc_sse_emit_movntdq_store_memoffset (compiler, offset, reg1, reg2);
        } else {
          orc_sse_emit_movdqa_store_memoffset (compiler, offset, reg1, reg2);
        }
      } else {
        orc_sse_emit_movdqu_store_memoffset (compiler, offset, reg1, reg2);
      }
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }

}

void orc_x86_emit_mov_sse_reg_reg (OrcCompiler *compiler, int reg1, int reg2)
{
  if (reg1 == reg2) {
    return;
  }

  orc_sse_emit_movdqu (compiler, offset, reg1, reg2);
#if 0
  ORC_ASM_CODE(compiler,"  movdqa %%%s, %%%s\n", orc_x86_get_regname_sse(reg1),
        orc_x86_get_regname_sse(reg2));

  *compiler->codeptr++ = 0x66;
  orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x6f;
  orc_x86_emit_modrm_reg (compiler, reg1, reg2);
#endif
}

void orc_x86_emit_mov_reg_sse (OrcCompiler *compiler, int reg1, int reg2)
{
  ORC_ASM_CODE(compiler,"  movd %%%s, %%%s\n", orc_x86_get_regname(reg1),
      orc_x86_get_regname_sse(reg2));
  *compiler->codeptr++ = 0x66;
  orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x6e;
  orc_x86_emit_modrm_reg (compiler, reg1, reg2);
}

void orc_x86_emit_mov_sse_reg (OrcCompiler *compiler, int reg1, int reg2)
{
  ORC_ASM_CODE(compiler,"  movd %%%s, %%%s\n", orc_x86_get_regname_sse(reg1),
      orc_x86_get_regname(reg2));
  *compiler->codeptr++ = 0x66;
  orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x7e;
  orc_x86_emit_modrm_reg (compiler, reg2, reg1);
}

void
orc_sse_set_mxcsr (OrcCompiler *compiler)
{
  int value;

  ORC_ASM_CODE(compiler,"  stmxcsr %d(%%%s)\n",
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      orc_x86_get_regname_ptr(compiler, compiler->exec_reg));
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0xae;
  orc_x86_emit_modrm_memoffset_old (compiler, 3,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]), compiler->exec_reg);

  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      compiler->exec_reg, compiler->gp_tmpreg);

  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_C1]),
      compiler->exec_reg);

  value = 0x8040;
  ORC_ASM_CODE(compiler,"  orl $%d, %%%s\n", value,
      orc_x86_get_regname(compiler->gp_tmpreg));
  orc_x86_emit_rex(compiler, 4, 0, 0, compiler->gp_tmpreg);
  *compiler->codeptr++ = 0x81;
  orc_x86_emit_modrm_reg (compiler, compiler->gp_tmpreg, 1);
  *compiler->codeptr++ = (value & 0xff);
  *compiler->codeptr++ = ((value>>8) & 0xff);
  *compiler->codeptr++ = ((value>>16) & 0xff);
  *compiler->codeptr++ = ((value>>24) & 0xff);

  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      compiler->exec_reg);

  ORC_ASM_CODE(compiler,"  ldmxcsr %d(%%%s)\n",
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      orc_x86_get_regname_ptr(compiler, compiler->exec_reg));
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0xae;
  orc_x86_emit_modrm_memoffset_old (compiler, 2,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]), compiler->exec_reg);
}

void
orc_sse_restore_mxcsr (OrcCompiler *compiler)
{
  ORC_ASM_CODE(compiler,"  ldmxcsr %d(%%%s)\n",
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_C1]),
      orc_x86_get_regname_ptr(compiler, compiler->exec_reg));
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0xae;
  orc_x86_emit_modrm_memoffset_old (compiler, 2,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_C1]), compiler->exec_reg);
}

