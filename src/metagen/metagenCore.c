#include "metagen\metagenCore.h"
#include "os\core\osCore.h"

bool metagenHandleEmbedFile(BaseArena *arena, MetagenOutput *output, CTokArray nextToks)
{
    if (nextToks.len >= 7) // (mode, path) = 5 tokens
    {
        if (METAGEN_TOK_MATCH_KIND(nextToks.data[0], '('))
        {
            if (METAGEN_TOK_MATCH_KIND(nextToks.data[1], CTOK_IDEN))
            {
                str8 name = nextToks.data[1].lexeme;

                if (METAGEN_TOK_MATCH_KIND(nextToks.data[2], ','))
                {
                    if (nextToks.data[3].kind == CTOK_STR_LIT)
                    {
                        str8 path = baseCLexerGetStr8RepFromTokLexeme(arena, nextToks.data[3]);
                        U8Array contents = OSFileReadAll(arena,  path);

                        if (METAGEN_TOK_MATCH_KIND(nextToks.data[4], ','))
                        {
                            if (METAGEN_TOK_MATCH_KIND(nextToks.data[5], CTOK_IDEN))
                            {
                                bool bin = METAGEN_TOK_MATCH_LEXEME(nextToks.data[5], STR8_LIT("bin")) ||
                                           METAGEN_TOK_MATCH_LEXEME(nextToks.data[5], STR8_LIT("binary"));

                                Str8List entryList = {0};

                                Str8ListPushLastFmt(arena, &entryList, "// metagen_embedfile(%S, %S), line: %lld\n", name, path, nextToks.data[0].pos.line);
                                Str8ListPushLastFmt(arena, &entryList, "U8Array %S = \n", name);
                                Str8ListPushLastFmt(arena, &entryList, "{");

                                if (bin)
                                {
                                    Str8ListPushLastFmt(arena, &entryList, "\t.data=(u8[%lld])\n", contents.len);
                                    Str8ListPushLastFmt(arena, &entryList, "\t{");

                                    const u64 width = 16; // bytes

                                    for(u64 i = 0; i < contents.len; i++)
                                    {
                                        if (i % width == 0)
                                        {
                                            Str8ListPushLastFmt(arena, &entryList, "\n\t\t");
                                        }

                                        Str8ListPushLastFmt(arena, &entryList, "%3lld,", (u64)contents.data[i]);
                                    }
                                    Str8ListPushLastFmt(arena, &entryList, "\n\t},\n");
                                }
                                else
                                {
                                    Str8ListPushLastFmt(arena, &entryList, "\t.data=(u8*)\n", contents.len);
                                    Str8ListPushLastFmt(arena, &entryList, "\t\"");
                                    for(u64 i = 0; i < contents.len; i++)
                                    {
                                        if (contents.data[i] == '\n')
                                        {
                                            Str8ListPushLastFmt(arena, &entryList, "\\n\"\n\t\"");
                                        }
                                        else if (contents.data[i] == '\r')
                                        {
                                            Str8ListPushLastFmt(arena, &entryList, "\\r");
                                        }
                                        else if (contents.data[i] == '\t')
                                        {
                                            Str8ListPushLastFmt(arena, &entryList, "\\t");
                                        }
                                        else if (contents.data[i] == '\\')
                                        {
                                            Str8ListPushLastFmt(arena, &entryList, "\\\\");
                                        }
                                        else if (contents.data[i] == '\"')
                                        {
                                            Str8ListPushLastFmt(arena, &entryList, "\\\"");
                                        }
                                        else
                                        {
                                            Str8ListPushLastFmt(arena, &entryList, "%c", contents.data[i]);
                                        }
                                    }
                                    Str8ListPushLastFmt(arena, &entryList, "\",\n");
                                }
                                Str8ListPushLastFmt(arena, &entryList, "\t.len=%lld,\n", contents.len);
                                Str8ListPushLastFmt(arena, &entryList, "};\n");

                                Str8ListPushLast(arena, &output->header.embeds, Str8ListJoin(arena, &entryList, null));
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

Str8List metagenFindFilesToProcess(BaseArena *arena, str8 path)
{
    Str8List ret = {0};
    if (baseStringsStrContains(path, '.') && OSPathExists(path))
    {
        Str8ListPushLast(arena, &ret, path);
    }
    else
    {
        OSFileFindIter *iter = OSFindFileBegin(arena, path, null);
        if (iter != null)
        {;
            for(OSFileInfo *fileInfo = null; OSFindFileNext(arena, iter, fileInfo); )
            {
                if (fileInfo->attrs & OS_FILEATTR_DIR)
                {
                    
                }
                else
                {

                }
            }

            OSFindFileEnd(iter);
        }
    }

    return ret;
}