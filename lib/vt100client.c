// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/player/vt100client.c
// Author:      Gnomi (10.08.2007)
// Description: Eine getrennte Eingabezeile

#include <config.h>
#include <files.h>
#include <input_to.h>
#include <invis.h>
#include <interactive_info.h>
#include <message.h>
#include <portal.h>
#include <telnet.h>
#include <term.h>
#include <time.h>

#include "player.h"

#define MAX_COMMAND_LENGTH 998
#define NUM_INPUT_LINES    2
#define REMOVE_COLOURS(str) regreplace((str),"\e[^A-Za-z]*[A-Za-z]","",1)

#define NUMPAD_WALKING        this_object()->query_client_option(CLIENT_VT100_NUMPAD)

nosave int lines, width;  // Speichern wir mal hier, fuer schnelleren Zugriff.
nosave string nl_pending; // Ein \n muss noch ausgegeben werden.
nosave string cur_prompt=""; // Der aktuelle Prompt (input_to oder normal)
nosave int cur_prompt_length;
nosave string cur_cmd=""; // Die aktuelle Eingabe
nosave int cur_pos;       // Cursorposition innerhalb cur_cmd
nosave mixed *cur_hist;   // Die letzten Befehle
nosave int cur_hist_pos;  // Die Position, wenn die History durchgelaufen wird.
nosave int last_tab;      // 1, wenn zuletzt eine erfolglose Vervollstaendigung
                          // stattfand.
nosave int active;          // 1: Aktiv, 0: Suspended, -1: Verzoegert


// Prototypen aus input_to:
protected int clean_input_tos();
protected void clear_input_tos();
nomask object query_pending_input_to();
protected mixed query_pending_input_to_prompt();
protected int execute_input_to(string cmd);


private void show_prompt();
varargs private void move_prompt(int checkboundary)
{
    int skipfwd = cur_prompt_length+cur_pos;
    int col;
    
    if(!width)
        return;

    col = skipfwd%width;
    
    if(checkboundary==1 && !col || // Der Cursor wurde nach rechts verrueckt
       checkboundary==2 && col+1==width) // Nach links
        show_prompt();
    else
        efun::tell_object(this_object(),
            VT_POS(1+col,lines-NUM_INPUT_LINES+1+
                min(NUM_INPUT_LINES-1,skipfwd/width)));
}

protected string calc_trenner_info()
{
    string vis = (!IS_INVIS(this_object())) && "";
    return sprintf("_____%s%s%s___(AP: %d, %s: %d)___",
        vis || "(",
        this_object()->query_real_cap_name() || 
        capitalize(this_object()->query_real_name()),
        vis || ")",
        this_object()->query_hp(),
        this_object()->query_sp_short_name(),
        this_object()->query_sp());
}

private void show_prompt()
{
    int line = max((cur_prompt_length+cur_pos)/width,NUM_INPUT_LINES-1);
    string text;
    mixed* t = timearray(time());
    string trenner = calc_trenner_info();
    string trennerrechts = sprintf("%s___%02d:%02d",
        MUD_NAME, t[TM_HOUR], t[TM_MIN]);

    if(sizeof(trenner)+sizeof(trennerrechts)<=width)
        trenner += "_"*(width-sizeof(trenner)-sizeof(trennerrechts))+trennerrechts;
    
    text = cur_cmd[(line-NUM_INPUT_LINES+1)*width-cur_prompt_length..(line+1)*width-1-cur_prompt_length];
    if(line+1 == NUM_INPUT_LINES)
        text = cur_prompt + text;
    
    // Scroll-Regionen aufheben:
    efun::tell_object(this_object(),
        VT_SPLIT(1, lines)
        VT_POS(1,lines-NUM_INPUT_LINES) +
        trenner +
        VT_SPLIT(lines-NUM_INPUT_LINES+1, lines)
        VT_POS(1,lines-NUM_INPUT_LINES+1)
        VT_DEL_EOS + text);

    if(sizeof(cur_cmd)>cur_pos || (cur_prompt_length+cur_pos)%width==0)
        move_prompt();        
}

void receive_message_low(string msg)
{
    if(extern_call() && 
        object_name(previous_object()) != __MASTER_OBJECT__)
            return;

    if(interactive() && sizeof(msg))
    {
        if(active>0)
        {
            string msg_nc;
            
            efun::tell_object(this_object(), 
                VT_SPLIT(1, lines-1-NUM_INPUT_LINES) VT_RESTORE_CUR);
            
            if(nl_pending)
                efun::tell_object(this_object(), nl_pending);
        
            msg_nc = sizeof(msg) && REMOVE_COLOURS(msg);
            if(sizeof(msg_nc) && msg_nc[<1]=='\n')
            {
                int lastnl = strrstr(msg,"\n");
                nl_pending = "\n";
                msg = msg[0..lastnl-1]+msg[lastnl+1..<1];
            }
            else
                nl_pending = 0;
                
            efun::tell_object(this_object(),
                msg + VT_SAVE_CUR VT_SPLIT(lines-NUM_INPUT_LINES+1, lines));
            move_prompt();
        }
        else
            efun::tell_object(this_object(), msg);
    }
}

protected string|string* complete_command(string cmd); // Aus der tippse.

private int completion()
{
    string|string* result = complete_command(cur_cmd[..cur_pos-1]);
    if (!result)
    {
        efun::tell_object(this_object(), "\a");
        return 0;
    }

    if (stringp(result) && sizeof(result))
    {
        cur_cmd[..cur_pos-1] = result;
        cur_pos += sizeof(result) - cur_pos;
        show_prompt();
        return 0;
    }

    if (pointerp(result))
    {
        // Die Moeglichkeiten anzeigen:
        if(last_tab && this_object()->query_wiz_level())
            receive_message_low(sprintf("%#-78s\n",implode(result,"\n")));
        return 1;
    }
}

nosave string pending_input = 0;
protected string *query_history_commands();
protected void add_history(string str, string orig);
private void restore_prompt();

private void eval_prompt(mixed prompt)
{
    int nlpos;
    
    cur_prompt = funcall(prompt) || "";
    
    nlpos = strrstr(cur_prompt,"\n");
    if(nlpos>=0)
        receive_message_low(cur_prompt[0..nlpos]);

    cur_prompt = cur_prompt[nlpos+1..<1];
    cur_prompt_length = sizeof(REMOVE_COLOURS(cur_prompt||"")-"\b");

    if(active>0)
        show_prompt();
    else if(!active)
        receive_message_low(cur_prompt);
}

private void quick_exec(string cmd, string str)
{
    receive_message_low("> "+str+"\n");
    call_out(#'restore_prompt, 0); // Im Fehlerfalle
    if(!strstr(object_name(environment())||"", PORTAL_ROOM_DIR))
        environment()->cmd(cmd);
    else
        efun::command(cmd);
    remove_call_out(#'restore_prompt);
}

private void cmd_input(string str)
{
/*
#ifdef Orbit
    if(this_object()->query_real_name()=="gnomilein")
    receive_message_low(sprintf("%Q, %Q\n",str, pending_input));
#endif
*/
    int newprompt = clean_input_tos();
    
    foreach(int ch: str)
    {
        if(ch<0)
            ch+=256;

        if(ch == 0)
            continue;

        if(ch != '\t')
            last_tab = 0;
        
        if(pending_input)
        {
            pending_input += to_string(({ch}));
            switch(pending_input)
            {
                case "\e[":
                case "\eO":
                case "\e~":
                    continue; // Weiter sammeln
                
                case "\eOv": // Numpad rechts
                case "\e[2C":  // Shift+Rechts
                    if(NUMPAD_WALKING)
                    {
                        quick_exec("o","osten");
                        break;
                    }
                    // Fallthrough
                case "\e[C": //rechts
                case "\eOC":
                case "\eC":
                    if(cur_pos<sizeof(cur_cmd))
                    {
                        cur_pos++;
                        move_prompt(1);
                    }
                    break;
                
                case "\eOt": // Numpad links
                case "\e[2D":  // Shift+Links
                    if(NUMPAD_WALKING)
                    {
                        quick_exec("w","westen");
                        break;
                    }
                    // Fallthrough
                case "\e[D": //links
                case "\eOD":
                case "\eD":
                    if(cur_pos>0)
                    {
                        cur_pos--;
                        move_prompt(2);
                    }
                    break;
                
                case "\eOw": // Numpad Pos1/Home
                    if(NUMPAD_WALKING)
                    {
                        quick_exec("nw","nordwesten");
                        break;
                    }
                    // Fallthrough
                case "\e[1~": // Pos1/Home
                    if(cur_pos>0)
                    {
                        int oldpos = cur_pos;
                        cur_pos=0;

                        if(oldpos>=NUM_INPUT_LINES*width)
                            show_prompt();
                        else
                            move_prompt();
                    }
                    break;
                    
                case "\eOq": // Numpad Ende
                    if(NUMPAD_WALKING)
                    {
                        quick_exec("sw","südwesten");
                        break;
                    }
                    // Fallthrough
                case "\e[4~": // Ende
                    if(cur_pos<sizeof(cur_cmd))
                    {
                        int oldpos = cur_pos;
                        cur_pos = sizeof(cur_cmd);
                        
                        if(cur_pos>=NUM_INPUT_LINES*width &&
                           oldpos/width != cur_pos/width)
                            show_prompt();
                        else
                            move_prompt();
                    }
                    break;
                
                case "\eOx": // Numpad hoch
                case "\e[2A":  // Shift+Hoch
                    if(NUMPAD_WALKING)
                    {
                        quick_exec("n","norden");
                        break;
                    }
                    // Fallthrough
                case "\e[A": //hoch
                case "\eOA":
                case "\eA":
                    if(!cur_hist)
                    {
                        cur_hist = query_history_commands();
                        cur_hist_pos = sizeof(cur_hist);
                        if(query_pending_input_to())
                            cur_hist = map(cur_hist, (: "!"+$1 :));
                        cur_hist += ({cur_cmd});
                    }
                    else if(cur_hist_pos == sizeof(cur_hist)-1)
                        cur_hist[<1] = cur_cmd;
                    
                    if(cur_hist_pos>0)
                    {
                        cur_hist_pos--;
                        cur_cmd = cur_hist[cur_hist_pos];
                        cur_pos = sizeof(cur_cmd);
                        show_prompt();
                    }
                    break;
                
                case "\eOr": // Numpad runter
                case "\e[2B": //Shift+Runter
                    if(NUMPAD_WALKING)
                    {
                        quick_exec("s","süden");
                        break;
                    }
                    // Fallthrough
                case "\e[B": // runter
                case "\eOB":
                case "\eB":
                    if(cur_hist && cur_hist_pos < sizeof(cur_hist)-1)
                    {
                        cur_hist_pos++;
                        cur_cmd = cur_hist[cur_hist_pos];
                        cur_pos = sizeof(cur_cmd);
                        show_prompt();
                    }
                    break;
                
                
                case "\e[3~": // Entfernen
                    if(cur_pos<sizeof(cur_cmd))
                    {
                        cur_cmd[cur_pos..cur_pos]="";
                        show_prompt();
                    }
                    break;

                case "\eOu": // Numpad 5
                case "\e[E": 
                    if(NUMPAD_WALKING)
                        quick_exec("b","betrachte");
                    break;
                case "\eOy": // Numpad Bild hoch
                    if(NUMPAD_WALKING)
                        quick_exec("no","nordosten");
                    break;
                case "\eOs": // Numpad Bild runter
                    if(NUMPAD_WALKING)
                        quick_exec("so","südosten");
                    break;
                case "\eOM": // Numpad Enter
                    pending_input = 0;
                    cmd_input("\r");
                    return;
                case "\eOo": // Numpad /
                case "\eOQ": // Numpad /
                    if(NUMPAD_WALKING)
                        quick_exec("\\sp","spielstand");
                    break;
                case "\eOj": // Numpad *
                case "\eOR": // Numpad *
                    if(NUMPAD_WALKING)
                        quick_exec("i","ausrüstung");
                    break;
                case "\eOm": // Numpad -
                case "\eOS": // Numpad -
                    if(NUMPAD_WALKING)
                        quick_exec("h","hoch");
                    break;
                case "\eOk": // Numpad +
                case "\eOl": // Numpad +
                    if(NUMPAD_WALKING)
                        quick_exec("r","runter");
                    break;
                case "\eOn": // Numpad ,
                    break;
                case "\eOp": // Numpad 0
                    if(NUMPAD_WALKING)
                        quick_exec("a","ausgang");
                    break;
                
#if __VERSION__ <= "3.5.2"
                case "\xe2\x82": // UTF8-Anfang
                    continue;
#endif
                    
                default:
                    if(pending_input[0] != '\e')
                    {
#if __VERSION__ > "3.5.2"
                        string erg = pending_input;
#else
                        string erg = convert_umlaute(pending_input);
#endif
                        if(sizeof(cur_cmd)+sizeof(erg)>MAX_COMMAND_LENGTH)
                            efun::tell_object(this_object(),"\a");
                        else
                        {
                            cur_cmd[cur_pos..cur_pos-1] = erg;
                            cur_pos += sizeof(erg);
                            if(cur_pos==sizeof(cur_cmd) && (cur_prompt_length+sizeof(cur_cmd))%width)
                                efun::tell_object(this_object(), erg);
                            else
                                show_prompt();
                        }
                        break;
                    }
                    else  if(sizeof(pending_input)>2 && pending_input[1]=='[' &&
                        pending_input[2]>='0' && pending_input[2]<='9' &&
                        ((pending_input[<1]>='0' && pending_input[<1]<='9') || pending_input[<1]==';'))
                            continue;
            }
            pending_input = 0;
        }
        else switch(ch)
        {
            case 3: /* Ctrl+C*/
                if(sizeof(cur_cmd))
                {
                    cur_cmd = "";
                    cur_pos = 0;
                    show_prompt();
                }
                break;
                
            case '\e':
                pending_input = "\e";
                break;
                
            case '\t':
                // Geht nicht in einem input_to.
                if(query_pending_input_to() && (!sizeof(cur_cmd) || cur_cmd[0]!='!'))
                {
                    efun::tell_object(this_object(), "\a");
                    last_tab = 0;
                }
                else
                    last_tab = completion();
                break;
        
            case '\b':
            case 127:
                if(cur_pos)
                {
                    cur_pos--;
                    cur_cmd[cur_pos..cur_pos]="";
                    show_prompt();
                }
                break;

            case '\n':
                if(!sizeof(cur_cmd))
                    break; //Ignorieren.
                // Fall through.
            case '\r':
            {
                string cmd = cur_cmd;
                
                receive_message_low(cur_prompt+cur_cmd+"\n");
                cur_prompt = "";
                cur_prompt_length = 0;
                cur_hist = 0;
                cur_cmd = "";
                cur_pos = 0;
                show_prompt();
                
                call_out(#'restore_prompt, 0); // Im Fehlerfalle

                // Den Befehl erst am Ende ausfuehren, da
                // Fehler auftreten koennen.
                
                if(!strstr(object_name(environment())||"", PORTAL_ROOM_DIR))
                {
                    add_history(cmd, cmd);
                    environment()->cmd(cmd);
                }
                else if(sizeof(cmd) && cmd[0]=='!') // Bang simulieren.
                {
                    cmd = cmd[1..<1];
                    efun::command(cmd);
                    
                    if(this_object() &&
                        efun::find_input_to(this_object(), #'cmd_input)<0)
                    {
                        // Es wurde kein neues gestartet?
                        newprompt = 1;
                    }
                }
                else
                {
                    if(!execute_input_to(cmd))
                        efun::command(cmd);
                }

                remove_call_out(#'restore_prompt);
                if(!newprompt)
                    return;
                break;
            }
        
#if __VERSION__ <= "3.5.2"
            case 195: // Multi-Byte Kodierungen
            case 226:
                pending_input = to_string(({ch}));
                break;
#endif
                
            default:
            {
#if __VERSION__ > "3.5.2"
                string erg = to_string(({ch}));
#else
                string erg = convert_umlaute(to_string(({ch})));
#endif
                if(sizeof(cur_cmd)+sizeof(erg)>MAX_COMMAND_LENGTH)
                    efun::tell_object(this_object(),"\a");
                else
                {
                    cur_cmd[cur_pos..cur_pos-1] = erg;
                    cur_pos += sizeof(erg);
                    if(cur_pos==sizeof(cur_cmd) && (cur_prompt_length+sizeof(cur_cmd))%width)
                        efun::tell_object(this_object(), erg);
                    else
                        show_prompt();
                }
                
                break;
            }
        }
    }

#if __EFUN_DEFINED__(query_input_pending)
    if(this_interactive() && !efun::query_input_pending(this_object()))
#else
    if(this_interactive() == this_object() && !efun::interactive_info(this_object(), II_INPUT_PENDING))
#endif
    {
        if(newprompt)
            eval_prompt(query_pending_input_to_prompt() || set_prompt(0, this_object()));
        
        if(active)
            efun::input_to(#'cmd_input,
                INPUT_CHARMODE|INPUT_NOECHO|INPUT_IGNORE_BANG);
    }
}

// Driver-Hook H_PRINT_PROMPT
int print_prompt(mixed prompt)
{
    if (extern_call() && object_name(previous_object()) != __MASTER_OBJECT__)
        return 0;

    if(!this_object()->query_client_option(CLIENT_VT100) || !active)
        return 0;

    clean_input_tos();

    // Falls ein input_to laeuft, hat dieses Vorrang
    eval_prompt(query_pending_input_to_prompt() || prompt);
    
#if __EFUN_DEFINED__(query_input_pending)
    if(active && !efun::query_input_pending(this_object()))
#else
    if(active && interactive() && !efun::interactive_info(this_object(), II_INPUT_PENDING))
#endif
        efun::input_to(#'cmd_input, INPUT_CHARMODE|INPUT_NOECHO|INPUT_IGNORE_BANG);

    return 1;
}

private void restore_prompt()
{
    if(active>0 && !query_input_pending(this_object()))
        print_prompt(set_prompt(0, this_object()));
}

private void set_geometry(int rows, int cols)
{
    int ldiff = lines && (lines-rows);
    int rdiff = width && (width-cols);
    
    width = cols;
    lines = rows;
    
    if(ldiff>0)
    {
        // Wir haben weniger Zeilen, wir wissen nicht
        // wie sich der Text durch das Scrollen verhielt,
        // also loeschen wir alles und beginnen ganz oben.
        efun::tell_object(this_object(),
            VT_CLR_SCR
            VT_RESTORE_CUR
            VT_POS(1,1)
            VT_SAVE_CUR);
    }
    else if(ldiff<0 || rdiff)
    {
        // Wir haben mehr, wir setzen den Cursor aber
        // sicherheitshalber ans Ende des Fensters
        efun::tell_object(this_object(),
            VT_RESTORE_CUR
            VT_POS(1,lines-1-NUM_INPUT_LINES)
            VT_SAVE_CUR);
    }
    
    if(width<1)
        width = 1;
    
    show_prompt();
}

nomask static void start_numpad()
{
    efun::tell_object(this_object(), VT_ESC "="  VT_ESC "[?1061h"VT_ESC "[?66h");
}

nomask static void stop_numpad()
{
    efun::tell_object(this_object(), VT_ESC ">" VT_ESC "[?1061l" VT_ESC "[?66l");
}

nomask static void start_mudclient()
{
    mixed sb;
    int rows, cols;
    int naws;
    
    //if(!active && this_object()->uses_webmud())
    if(this_object()->uses_webmud() || this_object()->uses_webmud3())
    {
        active = 0;
        return;
    }

    naws = this_object()->query_telnet(TELOPT_NAWS,&sb);
    remove_call_out(#'start_mudclient);
    
    // 'IAC DO NAWS' wurde abgeschickt oder noch nicht initialisiert.
    if(((naws & 0x03) == 0x03 || !naws) && !active)
    {
        // Wir warten mal auf das NAWS.
        active = -1;
        call_out(#'start_mudclient, 1);
        return;
    }
    
    active = 1;

    if(naws && pointerp(sb))
    {
        cols = sb[0];
        rows = sb[1];
    }
    else
    {
        rows = 24; // Default
        cols = 79;
    }
    
    // Cursor sichern und Text hochscrollen
    efun::tell_object(this_object(), VT_SAVE_CUR VT_SPLIT(1,rows) VT_RESTORE_CUR
    VT_ESC "D" VT_ESC "D" VT_ESC "D" VT_ESC "D" VT_UPP(4)
    VT_SAVE_CUR);

    if(this_object()->query_client_option(CLIENT_VT100_NUMPAD))
        start_numpad();

    lines = 0; // Damit set_geometry nicht den Bildschirm loescht.
        set_geometry(rows, cols);
}

nomask static varargs void stop_mudclient(int dontclear)
{
    if(active<0)
    {
        remove_call_out(#'start_mudclient);
        active = 0;
        return;
    }

    active = 0;

    if(this_object()->query_client_option(CLIENT_VT100_NUMPAD))
        stop_numpad();
    
    // Scroll-Regionen aufheben:
    efun::tell_object(this_object(),
        VT_SPLIT(1, lines)
        VT_RESTORE_CUR
        VT_DEL_EOS
        +(nl_pending||""));
    nl_pending = 0;
    efun::remove_input_to(this_object(), #'cmd_input);
    if(!dontclear)
        clear_input_tos();
}

nomask void suspend_mudclient()
{
    if(active && interactive())
        stop_mudclient(1);
}

nomask void restart_mudclient()
{
    if(this_object()->query_client_option(CLIENT_VT100) && !active && interactive())
        start_mudclient();
}

private void window_size_changed(string controller, object pl, int cols, int rows)
{
    if(this_object()->query_client_option(CLIENT_VT100) && active)
    {
        if(active<0)
            start_mudclient();
        else
            set_geometry(rows, cols);
    }
}

// Wird von sefun::cat und sefun::tail aufgerufen.
nomask int show_func_result(closure fun)
{
    int suspended;
    int result;
    string err;
    
    if(strstr(object_name(previous_object()),"/secure/simul_efun/") &&
       (strstr(object_name(previous_object()), "/obj/zauberstab#") ||
        !present(previous_object(), this_object())))
        return 0;
    
    if(active && interactive())
    {
        efun::tell_object(this_object(), 
            VT_SPLIT(1, lines-1-NUM_INPUT_LINES) VT_RESTORE_CUR);
            
        if(nl_pending)
            efun::tell_object(this_object(), nl_pending);
        nl_pending = 0;
        suspended = 1;
    }
    
    err = catch(result = funcall(fun); reserve 5000);
    
    if(suspended && interactive())
    {
        efun::tell_object(this_object(),
            VT_SAVE_CUR VT_SPLIT(lines-NUM_INPUT_LINES+1, lines));
        move_prompt();
    }

    if(err)
        raise_error(err[1..]);

    return result;
}

nomask void update_prompt()
{
    if(this_object()->query_client_option(CLIENT_VT100) && interactive() && active>0)
        show_prompt();
}

protected void update_points_display()
{
    update_prompt();
}

protected int query_reserved_height()
{
    if(this_object()->query_client_option(CLIENT_VT100) && active)
        return NUM_INPUT_LINES+1;
}

int uses_vt100client()
{
    if(!extern_call() || previous_object() == this_object()
     || !strstr(object_name(environment())||"", PORTAL_ROOM_DIR))
        return active;
}

void heart_beat()
{
    int* t = timearray(time());
    if(t[TM_SEC]<2 && active>0)
        show_prompt();
}

void create()
{
    this_object()->add_controller("notify_window_size", #'window_size_changed);
}
