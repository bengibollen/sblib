// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/control.c
// Description: Der control-Container
//              Empfaengt Nachrichten und leitet sie weiter
//              an angemeldete Controller
// Author:      Kurdel (19.8.99)

// Modified by: Mammi  (1.11.99) nur blueprints koennen sich anmelden,
//                               save/restore.

/******************************************************************************

In einem master wie zb /apps/control ist es notwendig, dass
- in save() des masters ein prepare_save() vor dem save_object aufgerufen wird
- in restore() des masters ein finalize_restore() nach dem restore_object
  aufgerufen wird.
- im create ein restore aufrufen (s.o.)
- es wird empfohlen, den Master mit *_renewal Funktionen auszustatten,
  in prepare_renewal ein save() und in finish_renewal ein 
  transfer_temporary_controller(neu) aufzurufen, damit die temopraeren
  Controller gerettet werden.

/apps/control (oder besser CONTROL, definiert in <apps.h>) dient dazu,
mudweite Ereignisse ueber das Controllerkonzept mitzuteilen.
/i/tools/control.c liefert die zentrale Funktionalitaet, damit
diese in control-Proxies eingesetzt werden kann.  In der folgenden Doku
wird von CONTROL ausgegangen, kann aber auch ein control-Proxy sein.

EINSCHRAENKUNG:
    Man kann nur Blueprints (als Objekt oder ueber ihren Filenamen) anmelden,
    keine Clones und keine Closures.

Im Blueprint kann man dann aber seine eigenen Clones wiederum anmelden und
so die Messages weiter verteilen.

Damit man CONTROL korrekt benutzen kann, gerade wenn ein Objekt auch als
Clone vorhanden sein koennte, schreibt man in die Funktion, die das Objekt
anmelden soll (z.b. im create()), folgende Zeilen:

    if (clonep())
    {
        // Damit wird der Clone beim Blueprint angemeldet:
        __FILE__->add_controller("<funktion>",this_object());
    }
    else
    {
        // Damit meldet sich das/der Blueprint bei CONTROL an:
        CONTROL->add_controller("<funktion>",__FILE__);
    }

(Nicht vergessen: <apps.h> includen!)

Damit hat sich das Objekt korrekt ueber sein(en) Blueprint bei CONTROL
fuer die Funktion <funktion> angemeldet. <funktion> wird natuerlich nur
aufgerufen, wenn sie mit "notify_", "forbidden_" oder "allowed_" beginnt,
das sollte klar sein ;)

Nun muss man, falls Euer Objekt auch ein Clone sein kann, nur noch dafuer
sorgen, dass vom Blueprint aus die Clones informiert werden. Das geht
z.b. bei notify-Funktionen so:

void notify_meine_funktion(<argumente>)
{
    if (!clonep())
    {
        notify("meine_funktion",<argumente>);
        return;
    }
    <implementierung des eigentlichen Notifys>
}

------------------------------ SCHNIPP -------------------------------
// Als Beispiel wollen wir mal mitbekommen, wenn ein Spieler ein Raetsel
// loest:

#include <apps.h>
void create()
{
    // ... irgendwelche set_-Funktionen...

    if (clonep())
        __FILE__->add_controller("notify_set_quest",this_object());
    else
        CONTROL->add_controller("notify_set_quest",__FILE__);

    // weitere set_-Funktionen usw...
}

// In Eurem Objekt implementiert ihr jetzt noch Eure Funktion, z.B:

#include <quest.h> // braucht man fuer Q_SOLVED
void notify_set_quest(string quest,int points,object who)
{
    if (!clonep())
    {
        notify("set_quest",quest,points,who);
        return;
    }
    if (points == Q_SOLVED)
        tell_object(who,wrap(Der()+" gratuliert Dir herzlich zum Loesen des "
            +"Raetsels "+capitalize(quest)+"!"));
}
------------------------------- SCHNAPP ------------------------------

Es gibt einige Standardaktivitaeten, die ueber CONTROL abgefragt werden
koennen:

Bestimmte Tageszeiten
~~~~~~~~~~~~~~~~~~~~~
void notify_midnight()
void notify_begin_daylight()
void notify_highnoon()
void notify_begin_darkness()

Werden Raetsel/Spiele geloest, bzw. EP gesetzt?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void notify_set_quest(string quest,int points,object who)
void notify_set_game(string game,int points,object who)

Information, dass ein Spieler suizidet wurde
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void notify_delete_player(string real_name)

Event-Kanaele, die Spieler hoeren koennten (kein zbruell-,oder Admin-Kanaele)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void notify_event_<event_name>(object who,string msg, int flag, string wizmsg)

Beispiele:
Hexen-, Bruelle-, Todes-, Statue-, Login-, Logout-, Level- oder Engelkanal:
notify_event_hexen, notify_event_gebruell, notify_event_tod,
notify_event_statue, notify_event_login, notify_event_logout,
notify_event_level, notify_event_engel

Technische Realisierung der Event-Beobachtung
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Events, die beobachtet werden koennen, besitzen einen Level, der <= LVL_WIZ
ist. Welche Kanaele das sind steht in /static/adm/EVENTS

Beachte: der Kanalname wird in lower_case() angesprochen (also kein
notify_event_Gebruell sondern notify_event_gebruell)

Notabene
~~~~~~~~
Bitte benutzt diese Moeglichkeiten nicht exzessiv. Es ist nicht Sinn der Sache,
dass sich jetzt jeder sein Goettertool bastelt, das jedem jeden Kanal meldet
und jeder jeden Kanal bequem andauernd mitloggen kann. Dafuer ist der
Kurier-Puffer da.
Vornehmlich soll diese Moeglichkeit fuer Dinge genutzt werden, die Spielern
etwas bringen. So koennte z.B. ein Bestattungsinstitut die letzten Todesfaelle
auf diese Weise "erfahren", ein NPC, der als Raetselhilfe fungiert, koennte
zuverlaessig mitbekommen, wenn ein Spieler seine Hilfe nicht mehr benoetigt.
Ein Gilden-NPC koennte erzaehlen, was die Gildenmitglieder "neulich so von
sich gegeben haben", Emma koennte ihre Geruechtekueche verbessern usw.

******************************************************************************/

inherit "/i/item/control";

#include <touch.h>
#include <error.h>


// TODO do_error2 anpassen, damit TO oder Verursacher daran glauben muss
#define ERR(x) { do_error2(wrap(x),__FILE__, \
                 object_name(previous_object()),__LINE__); return 0; }

#define MAX_CONTROLLER_PER_NAME 200

private static mapping names = ([]); // load_name: ({ Objektliste })
private mapping _control_objects = ([]);

varargs int _internal_add_controller(mixed func, mixed contr)
{
    int ret = 0;

    if(!extern_call() || program_name(previous_object())
         ==program_name(this_object()))
    {
        int num;

        if(!pointerp(func))
            func = ({ func });

        if(objectp(contr) || closurep(contr))
        {
            foreach(string f: func)
                if(!::is_controller(f, contr))
                    num++;
        }

        ret = ::add_controller(func, contr);

        if(num)
        {
            string name = load_name(to_object(contr));

            if(!member(names, name))
                names[name] = ({ contr }) * num;
            else
                names[name] = names[name] - ({0}) + ({ contr }) * num;
        }
    }

    return ret;
}

// Damit man nur Blueprints anmelden kann, keine Closures und Clones,
// wird add_controller() aus /i/item/control ueberlagert:
// Liefert 1 zurueck, wenn add_controller geklappt hat, ansonsten 0
varargs int add_controller(mixed func, mixed ob)
{
    object obj;
    int ret;

    if ( !ob )
        ob = previous_object();

    if ( objectp(ob) )
        obj = ob;
    else if ( closurep(ob) )
        obj = to_object(ob);
    else if ( stringp(ob) )
    {
        if ( catch(obj = touch(ob,NO_LOG | NO_WRITE); publish) )
        return 0;
        if ( !obj )
            ERR(sprintf("Cannot load %s!",ob));
    }
    else
        ERR("add_controller: bad type to arg 2. Must be object|closure|string.");

    if(!stringp(ob))
    {
        string name = load_name(obj);
        if(member(names, name) && sizeof(names[name]-=({0})) >= MAX_CONTROLLER_PER_NAME)
            ERR("add_controller: Too many controllers registered for '" + name + "'.");
    }

    if ( ret = _internal_add_controller(func, ob) && stringp(ob))
    {
        call_out("save_control",10);
    }

    return ret;
}

varargs int delete_controller(mixed func, mixed ob)
{
    int ret;

    ob ||= previous_object();

    if ( ret = ::delete_controller(func,ob) )
    {
        if(stringp(ob))
        {
            call_out("save_control",10);
        }
        else if(ob)
        {
            string name = load_name(to_object(ob));
            int pos;

            foreach(int i: pointerp(func) ? sizeof(func) : 1)
                if(member(names, name) && (pos = member(names[name], ob))>=0)
                    names[name] = names[name][0..pos-1] + names[name][pos+1..<1];
        }
    }

    return ret;
}

void notify(string message, varargs mixed data)
{
    mixed value,to_delete;

    if (value = query_controller(message = "notify_"+message))
    {
        if ( !pointerp(value))
            value = ({ value });

        data = ({ message }) + expand(data) + ({ this_object() });
        to_delete = ({});

    foreach(mixed contr: value - ({0}))
        {
            if ( closurep(contr) 
                ? catch(apply(contr, data); publish)
                : catch(apply(#'call_other,contr, data); publish) )
            {
                // Merken, welcher Controller kaputt ist:
                to_delete += ({contr});
            }
        }
        // Fehlerhafte Controller austragen. Einmal Mist reicht.
        filter(to_delete, (: delete_controller($2, $1) :), message);
    }
}

// Aufruf im save() des Master (vor save_object)
void prepare_save()
{
   _control_objects = query_controller();
}

// Aufruf im restore des Masters. (nach restore_object)
void finalize_restore()
{
    foreach(string func, mixed contr: _control_objects)
        if(pointerp(contr))
            foreach(string ob: contr)
                ob && ::add_controller(func, ob);
            else if(contr)
                ::add_controller(func, contr);
   _control_objects = ([]);
}

// Aufruf ueber finish_renewal im Master.
protected void transfer_temporary_controller(object neu)
{
    if (!objectp(neu)) return;
    // Die temporaeren Controller uebermitteln
    foreach(string func, mixed contr: query_controller())
	if(pointerp(contr))
	    foreach(mixed ob: contr)
		(objectp(ob) || closurep(ob)) && 
		    neu->_internal_add_controller(func, ob);
	else if(objectp(contr) || closurep(contr))
	    neu->_internal_add_controller(func, contr);
}
