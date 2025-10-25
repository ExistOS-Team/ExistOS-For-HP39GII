
#define SYMDEF(x) asm volatile("b " #x);

void symtab_def()
{

    SYMDEF(api_vram_initialize);
    SYMDEF(api_vram_get_current);
    SYMDEF(api_vram_flush);
    SYMDEF(api_vram_clear);
    SYMDEF(api_vram_put_char);
    SYMDEF(api_vram_put_string);
    SYMDEF(api_vram_set_pixel);
    SYMDEF(api_vram_draw_HLine);
    SYMDEF(api_vram_draw_VLine);
    SYMDEF(api_vram_draw_line);
    SYMDEF(api_vram_fill_rect);
    SYMDEF(api_get_key);



    SYMDEF(snprintf);
    SYMDEF(remove);
    SYMDEF(rename);
    SYMDEF(strrchr);
    SYMDEF(atof);
    SYMDEF(strstr);
    SYMDEF(strcasecmp);
    SYMDEF(strdup);
    SYMDEF(system);

}