// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/item/control.c
// Description: Empfangen von Nachrichten, Verbieten/Erlauben von Aktionen
// Author:      Kurdel (05/97)

#include <control.h>

#pragma save_types
#pragma strong_types

private static mapping control_objects;

#define USE_BLUEPRINT_POINTERS

// Macht aus 0 previous_object() oder this_object(),
// entfernt ".c"-Endungen, wandelt MAP-Namen in Domain-Namen um.
mixed normalize_controller(mixed ob, int extcall)
{
#ifdef USE_BLUEPRINT_POINTERS
    if(closurep(ob) || objectp(ob))
	return ob;
    if(!ob)
	return extcall?previous_object():this_object();
#else
    if(closurep(ob))
	return ob;
    if(!ob)
	ob = extcall?previous_object():this_object();
    if(objectp(ob))
	return (!clonep(ob))?object_name(ob):ob;
#endif
    if(!stringp(ob))
	return 0; // Fehler
    ob = map2domain(ob,1) || ob;
    if(ob[<2..<1]==".c")
	return ob[0..<3];
    else
	return ob;
}

/*
FUNKTION: is_controller
DEKLARATION: int is_controller(string func [, object ob | string filename | closure cl])
BESCHREIBUNG:
Ist das Objekt 'ob' / die Datei 'filename' / die Closure 'cl' fuer
den Controller 'func' bei einem Objekt 'ding' angemeldet, dann liefert

    ding->is_controller(func, ob/filename/cl)

den Wert 1 zurueck, ansonsten 0.

Ist kein Objekt/Filename/Closure angegeben, ist der Aufrufer das Objekt.
(this_object() bei direkten Aufrufen, previous_object() bei Call-Other).
VERWEISE: add_controller, delete_controller, query_controller, forbidden, notify
GRUPPEN: controller
*/
varargs int is_controller(string func, mixed ob)
{
   mixed value,obj;
   obj = normalize_controller(ob, extern_call());
   if(!obj)
	raise_error("Invalid argument 2 to is_controller.\n");
   return control_objects && (value = control_objects[func]) &&
      (value == obj || (pointerp(value) && member(value, obj) !=-1));
}

/*
FUNKTION: add_controller
DEKLARATION: varargs int add_controller(mixed func [, object ob | string filename | closure cl])
BESCHREIBUNG:
Mit add_controller() meldet man sich bei einem Objekt 'ding' fuer einen
oder mehrere Controller an. Die Grundlagen des Controller-Mechanismus
werden in /doc/funktionsweisen/controller naeher erlaeutert.

Das Objekt 'ob' / die Datei 'filename' / die Closure 'cl' meldet sich
fuer ein oder mehrere Controller 'func' bei dem Objekt 'ding' mit

    ding->add_controller(func, ob/filename/cl)

an. Falls die Anmeldung geklappt hat, wird 1 zurueckgeliefert,
ansonsten 0.

Ist kein Objekt/Filename/Closure angegeben, ist der Aufrufer das Objekt.
(this_object() bei direkten Aufrufen, previous_object() bei Call-Other).

Wird in dem Objekt 'ding' der Controller 'func' ausgeloest, werden
alle angemeldeten Objekte / Dateinamen / Closures benachrichtigt.
Wie diese Benachrichtigung genau aussieht, ist von Controller zu
Controller verschieden und der jeweiligen Dokumentation zu entnehmen.

'func' darf ein String oder ein String-Array sein. Falls man dasselbe
Objekt / Datei / Closure fuer mehrere Controller anmelden moechte,
empfiehlt sich ein Aufruf mit einem Array statt mehrerer Aufrufe mit
je einem String als 'func'.

Objekte, Dateinamen und Closures koennen pro Controller jeweils nur
einmal angemeldet werden. Ist das Objekt bei Aufruf schon eingetragen,
passiert dies also nicht ein weiteres Mal.

Bei angemeldeten Objekten und Closures ist zu beachten, dass die Anmeldung
nur solange besteht, solange das angemeldete Objekt / die Closure noch
existiert. Dateinamen dagegen bleiben angemeldet, solange 'ding' existiert.

Bei einem Closure-Aufruf wird ein string Parameter vor den anderen Parametern
vorangestellt, die den Namen des Controllers (func) enthaelt.
Bei allen Controlleraufrufen wird this_object() bei den Parametern angehaengt,
was bei den Einzeldokumentationen zu den Controllern nicht immer explizit 
erwaehnt wird.

Per delete_controller() kann der Controller wieder abgemeldet werden.
VERWEISE: delete_controller, query_controller, is_controller, forbidden, notify
GRUPPEN: controller
*/
varargs int add_controller(string|string* func, object|string|closure ob)
{
    mixed value, obj;
    
    obj = normalize_controller(ob, extern_call());
    if(!obj)
	raise_error("Invalid argument 2 to add_controller.\n");
    
    if (func && (stringp(func) || pointerp(func)))
    {
        if (!control_objects)
            control_objects = ([]);
        if (stringp(func))
            func = ({ func });
        foreach(string msg: func)
        {
            if (value = control_objects[msg])
            {
                if (pointerp(value))
                {
                    if (member(value, obj) == -1)
                        control_objects[msg] += ({obj});
                    control_objects[msg] -= ({0});
                }
                else if (value != obj)
                    control_objects[msg] = ({value, obj});
            }
            else
                control_objects[msg] = obj;
        }
        return 1;
    }            
    return 0;
}

/*
FUNKTION: delete_controller
DEKLARATION: int delete_controller(mixed func [, object ob | string filename | closure cl])
BESCHREIBUNG:
Mit delete_controller() meldet man Controller bei einem Objekt 'ding' ab,
die zuvor mit add_controller() angemeldet wurden. Die Grundlagen des
Controller-Mechanismus werden in /doc/funktionsweisen/controller
naeher erlaeutert.

Das Objekt 'ob' / die Datei 'filename' / die Closure 'cl' meldet sich fuer
ein oder mehrere Controller 'func' bei dem Objekt 'ding' mit

    ding->delete_controller(func, ob/filename/cl)

ab. Falls die Abmeldung geklappt hat, wird 1 zurueckgeliefert,
ansonsten 0.

Die Bedeutung der Parameter wird bei add_controller() erklaert.
VERWEISE: query_controller, add_controller, is_controller, forbidden, notify
GRUPPEN: controller
*/
varargs int delete_controller(string|string* func, object|string|closure ob)
{
    mixed value,obj;
    int i, ret;

    obj = normalize_controller(ob, extern_call());
    if(!obj)
        raise_error("Invalid argument 2 to delete_controller.\n");

    if (control_objects && (stringp(func) || pointerp(func)))
    {
        ret = 1;
        if (stringp(func))
            func = ({func});
        foreach(string msg: func)
        {
            if (pointerp(value = control_objects[msg]))
            {
                control_objects[msg] -= ({obj,0});
                if (!i = sizeof(value = control_objects[msg]))
                    control_objects = m_delete(control_objects, msg);
                else if (i==1)
                    control_objects[msg] = value[0];
            }
            else if (value==obj)
                control_objects = m_delete(control_objects, msg);
            else
                ret = 0;
        }
        return ret;
    }            
}

/*
FUNKTION: query_controller
DEKLARATION: mixed query_controller([string func])
BESCHREIBUNG:
Mit query_controller() kann man saemtliche Objekte / Dateinamen / Closures
erfragen, die sich bei einem Objekt 'ding' per add_controller() fuer den
Controller 'func' angemeldet haben.

Der Aufruf

    ding->query_controller(func)

liefert die 'anmeldungen' von 'func' zurueck. Der Parameter 'func' ist
optional. Wenn nicht angegeben oder 0, erhaelt man ein Mapping mit allen
angemeldeten Controllern in dem Format ([ 'func' : 'anmeldungen' ])
oder 0, falls kein einziger Controller angemeldet ist.

'anmeldungen' kann sein:

 0        - Es ist nichts (mehr) angemeldet.
 Objekt   - Ein einzelnes Objekt ist angemeldet.
 Filename - Ein einzelner Dateiname ist angemeldet.
 Closure  - Eine einzelne Closure ist angemeldet.
 Array    - Die enthaltenen Objekte / Dateinamen / Closures sind angemeldet.
            Vorsicht: Das Array kann auch leer sein oder Nullen enthalten.

Die Funktion ist meist nur Intern oder zu Debugzwecken interessant.
Wenn man wissen moechte, ob ein bestimmter Controller angemeldet ist,
verwendet man dazu besser die Funktion is_controller().
VERWEISE: add_controller, delete_controller, is_controller, forbidden, notify
GRUPPEN: controller
*/
varargs mixed query_controller(string func)
{
    return func ? control_objects && control_objects[func]
       : control_objects;
}

mixed expand(mixed data)
{
    if (pointerp(data) && sizeof(data) == 1 && pointerp(data[0]))
        return data[0];
    return data;
}

/*
FUNKTION: forbidden
DEKLARATION: mixed forbidden(string message, varargs mixed * data)
BESCHREIBUNG:
In einem Objekt 'ding' wird der forbidden-Controller 'message' ausgeloest.
Wer sich bei 'ding' fuer diesen Controller angemeldet hat, kann damit eine
bestimmte Aktion verbieten. Die Grundlagen des Controller-Mechansimus
werden in /doc/funktionsweisen/controller naeher erlaeutert.

Der Aufruf

    ding->forbidden(message, [arg1, arg2, ...] )

loest in dem Objekt 'ding' den Controller "forbidden_<message>" aus.
<message> ist hierbei der Wert des uebergebenen Strings 'message'.
Der Rueckgabewert ist 0, wenn die Aktion nicht verboten wurde, ansonsten
ein beliebiger Wert != 0.

Es werden der Reihe nach alle, die sich bei 'ding' per add_controller()
fuer den Controller "forbidden_<message>" angemeldet haben, gefragt, ob
sie die Aktion verbieten wollen oder nicht.

Dateien werden zunaechst geladen und fortan wie Objekte behandelt.

Objekte erhalten den Aufruf

    Objekt->forbidden_<message>( [arg1, arg2, ...], ding)

Closures erhalten den Aufruf

    funcall(Closure, "forbidden_<message>", [arg1, arg2, ...], ding)

Ist der Rueckgabewert != 0, liefert forbidden() diesen Wert zurueck,
und die Aktion wird verboten. Ansonsten wird das naechste angemeldete
Objekt / Dateiname / Closure benachrichtigt.

Hat sich niemand bei 'ding' fuer den Controller "forbidden_<message>"
angemeldet oder hatten alle Aufrufe den Rueckgabewert 0, dann liefert
forbidden() auch 0 zurueck, die Aktion wird demnach nicht verhindert.

Wie die Funktion forbidden_<message>() bzw. die Closure genau auszusehen
hat, ist der Dokumentation des jeweiligen Controllers zu entnehmen. Dort
steht vor allem, welche Parameter dem Controller uebergeben werden, und
wie man den Grund fuer die verhinderte Aktion auszugeben hat.
VERWEISE: add_controller, query_controller, notify, allowed
GRUPPEN: controller
*/
mixed forbidden(string message, varargs mixed * data)
{
    mixed value, ret;
    int i, size;
    
    if (control_objects)
    {
        data = ({ message = "forbidden_" + message }) + expand(data) +
          ({ this_object() });
        if (stringp(value = control_objects[message]) || objectp(value))
            return apply(#'call_other, value, data);
        if (closurep(value) && to_object(value))
            return apply(value, data);
        if (pointerp(value))
        {
            size = sizeof(value  = control_objects[message] -= ({0}));
            for (i; i < size; i++)
                if (ret = (closurep(value[i]))?
		   (to_object(value[i])?apply(value[i], data):0) : 
                   (value[i] && apply(#'call_other, value[i], data)))
                   return ret;
        }
    }
}

/*
FUNKTION: allowed
DEKLARATION: int allowed(string message, varargs mixed * data)
BESCHREIBUNG:
In einem Objekt 'ding' wird der allowed-Controller 'message' ausgeloest.
allowed ist das Gegenstueck zu forbidden. Aktionen, die normalerweise nicht
moeglich sind, werden erlaubt. Die Grundlagen des Controller-Mechansimus
werden in /doc/funktionsweisen/controller naeher erlaeutert.

Der Aufruf

    ding->allowed(message, [arg1, arg2, ...] )

loest in dem Objekt 'ding' den Controller "allowed_<message>" aus.
<message> ist hierbei der Wert des uebergebenen Strings 'message'.
Der Rueckgabewert ist 1, wenn die Aktion erlaubt wird, ansonsten 0.

Es werden der Reihe nach alle, die sich bei 'ding' per add_controller()
fuer den Controller "forbidden_<message>" angemeldet haben, gefragt, ob
sie die Aktion erlauben wollen oder nicht.

Dateien werden zunaechst geladen und fortan wie Objekte behandelt.

Objekte erhalten den Aufruf

    Objekt->allowed_<message>( [arg1, arg2, ...], 'ding')

Closures erhalten den Aufruf

    funcall(Closure, "allowed_<message>", [arg1, arg2, ...], 'ding')

Ist der Rueckgabewert != 0, liefert allowed() diesen Wert zurueck,
und die Aktion wird erlaubt. Ansonsten wird das naechste angemeldete
Objekt / Dateiname / Closure benachrichtigt.

Hat sich niemand bei 'ding' fuer den Controller "allowed_<message>"
angemeldet oder hatten jeder Aufruf den Rueckgabewert 0, dann liefert
allowed() auch 0 zurueck, die Aktion wird demnach nicht erlaubt.

Wie die Funktion allowed_<message>() bzw. die Closure genau auszusehen
hat, ist der Dokumentation des jeweiligen Controllers zu entnehmen. Dort
steht vor allem, welche Parameter dem Controller uebergeben werden, und
wie man den Grund fuer die nicht erlaubte Aktion auszugeben hat.
VERWEISE: add_controller, query_controller, forbidden, notify
GRUPPEN: controller
*/
int allowed(string message, varargs mixed * data)
{
    mixed value, ret;
    int i, size;
    
    if (control_objects)
    {
        data = ({ message = "allowed_" + message }) + expand(data) +
          ({ this_object() });
        if (stringp(value = control_objects[message]) || objectp(value))
            return apply(#'call_other, value, data);
        if (closurep(value) && to_object(value))
            return apply(value, data);
        if (pointerp(value))
        {
            size = sizeof(value  = control_objects[message] -= ({0}));
            for (i; i < size; i++)
	    {
		if (!value[i])
		    continue;
                if (ret = (closurep(value[i]))?
		   (to_object(value[i])?apply(value[i], data):0) : 
                   apply(#'call_other, value[i], data))
                   return ret;
	    }
        }
    }
}

/*
FUNKTION: notify
DEKLARATION: void notify(string message, varargs mixed * data)
BESCHREIBUNG:
In einem Objekt 'ding' wird der notify-Controller 'message' ausgeloest.
Wer sich bei 'ding' fuer diesen Controller angemeldet hat, kann damit eine
bestimmte Aktion mitbekommen. Die Grundlagen des Controller-Mechansimus
werden in /doc/funktionsweisen/controller naeher erlaeutert.

Der Aufruf

    ding->notify(message, [arg1, arg2, ...] )

loest in dem Objekt 'ding' den Controller "notify_<message>" aus.
<message> ist hierbei der Wert des uebergebenen Strings 'message'.
Es gibt keinen Rueckgabewert.

Es werden der Reihe nach alle, die sich bei 'ding' per add_controller()
fuer den Controller "notify_<message>" angemeldet haben, informiert,
dass die Aktion durchgefuehrt wurde (oder werden wird).

Dateien werden zunaechst geladen und fortan wie Objekte behandelt.

Objekte erhalten den Aufruf

    Objekt->notify_<message>( [arg1, arg2, ...], ding)

Closures erhalten den Aufruf

    funcall(Closure, "notify_<message>", [arg1, arg2, ...], ding)

Welche Parameter die Funktion notify_<message>() bzw. die Closure genau
erhaelt, ist der Dokumentation des jeweiligen Controllers zu entnehmen.
VERWEISE: add_controller, query_controller, forbidden, allowed
GRUPPEN: controller
*/
void notify(string message, varargs mixed * data)
{
    mixed value;
    int i,size;
    
    if (control_objects)
    {
        data = ({ message = "notify_" + message }) + expand(data) +
          ({ this_object() });
        if (stringp(value = control_objects[message]) || objectp(value))
           apply(#'call_other, value, data);
        else if (closurep(value) && to_object(value))
           apply(value, data);
        else if (pointerp(value))
        {
           size = sizeof(value = control_objects[message] -= ({0}));
           for(i; i<size; i++)
              if (closurep(value[i]))
	      {
	         if(to_object(value[i]))
                    apply(value[i], data);
	      }
              else if(value[i])
                 apply(#'call_other, value[i], data);
        }
    }
}

/*
FUNKTION: modify
DEKLARATION: void modify(string message, mixed &parameter, varargs mixed * data)
BESCHREIBUNG:
In einem Objekt 'ding' wird der modify-Controller 'message' ausgeloest.
Wer sich bei 'ding' fuer diesen Controller angemeldet hat, kann damit
den Parameter einer Aktion aendern. Die Grundlagen des Controller-Mechanismus
werden in /doc/funktionsweisen/controller naeher erlaeutert.

Der Aufruf

    ding->modify(message, &parameter, [arg1, arg2, ...])

loest in dem Objekt 'ding' den Controller "modify_<message>" aus.
<message> ist hierbei der Wert des uebergebenen Strings 'message'.
Es gibt keinen Rueckgabewert. Stattdessen kann 'parameter' per Referenz
veraendert werden.

Es wird der Reihe nach allen, die sich bei 'ding' per add_controller()
fuer den Controller "modify_<message>" angemeldet haben, die Gelegenheit
gegeben, den per Referenz uebergebenen 'parameter' zu veraendern.

Dateien werden zunaechst geladen und fortan wie Objekte behandelt.

Objekte erhalten den Aufruf

    Objekt->modify_<message>(&parameter, [arg1, arg2, ...], ding)

Closures erhalten den Aufruf

    funcall(Closure, "modify_<message>", &parameter, [arg1, arg2, ...], ding)

Wie die Veraenderung am 'parameter' genau vorzunehmen ist, und welche
weiteren Argumente zusaetzlich uebergeben werden, ist der Dokumentation
des jeweiligen Controllers zu entnehmen.
VERWEISE: add_controller, query_controller, forbidden, allowed, notify
GRUPPEN: controller
*/
void modify(string message, mixed variable_data, varargs mixed * fixed_data)
{
    mixed value;

    if (control_objects)
    {
       message= "modify_"+message;

       value = control_objects[message];
       if(!value)
          return;
       else if(!pointerp(value))
          value = ({value});

       fixed_data = expand(fixed_data) + ({this_object()});
	  
       foreach(mixed ob: value)
          if (closurep(ob) && to_object(ob))
             apply(ob, message, &variable_data, fixed_data);
	  else if(objectp(ob) || stringp(ob))
             apply(#'call_other, ob, message, &variable_data, fixed_data);
    }
}

/*
FUNKTION: concerned
DEKLARATION: mixed concerned([int &prio,] string message, varargs mixed * data)
BESCHREIBUNG:
Durch Aufruf dieser Funktion wird gefragt, welches Objekt fuer eine Aktion,
die mit message gekennzeichnet ist, zustaendig ist.

Der Rueckgabewert kann 0 sein, dann haelt sich kein Objekt dafuer zustaendig.
Ansonsten ist er entweder ein Objekt oder eine Closure.

Je nach message muss data eine bestimmte Form haben.

concerned ruft in allen mit add_controller("concerned_"+message, other)
angemeldeten Objekten other die Funktion "concerned_"+message auf.
Alle entsprechend angemeldeten Closures other werden mit "concerned_"+message
als ersten Parameter und die Parameter aus data als weitere Parameter
aufgerufen.
Die Funktion/Closure sollte dann eine Prioritaet zurueckliefern, mit welcher
sie sich fuer diese Aktion zustaendig fuehlt. 0 bedeutet, dass sie nicht
zustaendig ist. Dasjenige Objekt/diejenige Closure mit der hoechsten
Prioritaet wird dann zurueckgeliefert.

Die Funktion/Closure sollte keinerlei Ausgaben oder sonstige Aktionen
bewirken. Nachdem ein Objekt/eine Closure ob ermittelt wurde,
koennen dann weitere Aktionen folgen, indem im Objekt ob eine andere Funktion
bzw. die Closure ob mit einem anderen Namen als ersten Parameter aufgerufen
werden.

Wurde prio angegeben, so wird dies als Mindestprioritaet fuer eine Antwort
angenommen. Falls es als Referenz uebergeben wurde, so wird die gefundene
Prioritaet dort gespeichert.

Als Beispiel dient hier ein Streichholz, was testen moechte, ob ein
Objekt ob sich lieber selber anzuenden will. Dazu wird in ob ein
concerned("strike", this_object()) aufgerufen. Wird ein Objekt
zurueckgeliefert, so wird ihm das anzuenden ueberlassen:

  if(zustaendig = ob->concerned("strike", this_object()))
  {
    if(closurep(zustaendig)) funcall(zustaendig,"do_strike",this_object(),ob);
    else zustaendig->do_strike(this_object(),ob);
  }

Eine Bombe wird sich selber als Controller fuer concerned_strike anmelden
und mit einer entsprechend hohen Prioritaet reagieren:

  int concerned_strike(object streichholz, mixed was)
  {
    return 100;
  }
  
  // Dass diese Funktion eine Zahl zurueckliefert, ist speziell
  // fuer do_strike. Bei anderen Controllern kann dies anders aussehen.
  int do_strike(object streichholz, object was)
  {
    send_message(MT_LOOK|MT_NOISE|MT_FEEL,MA_UNKNOWN, "KRAWUMM!\n");
    remove();
    return 1; // Erfolg
  }
  
Im create():

  add_controller("concerned_strike",this_object());
  
Merke: Man fragt mit concerned("strike",..). Die Funktion concerned im
Objekt ruft dann in den eingetragenen Objekten concerned_strike auf, was
man nie selbst tun sollte.
Normale Benutzung: ob1->add_controller(func, ob2) und in ob2 ist eine
Funktion (func) concerned_message(...) definiert, in dieser ist
ob1==previous_object()!

Zusaetzlicher Parameter fuer func ist this_object().
VERWEISE: add_controller, query_controller, notify, forbidden, allowed
GRUPPEN: controller
*/
mixed concerned(mixed prio, varargs mixed * data)
{
    string message;
    mixed value, ret;
    int i, size;
    mixed max_ob;
    
    if(stringp(prio))
    {
	message = prio;
	prio = 0;
    }
    else if(!sizeof(data))
	return;
    else
    {
	message = data[0];
	data = data[1..<1];
	if(prio<0)
	    prio = 0;
    }
    
    if (control_objects)
    {
        data = ({ message = "concerned_" + message }) + expand(data) +
          ({ this_object() });

	value = control_objects[message];
	if ((!value) || (closurep(value) && !to_object(value))) return 0;
	else if (!pointerp(value)) value = ({ value });
	else value = control_objects[message] -= ({0});
        size = sizeof(value);
        for (i; i < size; i++)
            if ((ret = closurep(value[i]) ? 
		apply(value[i], data) :
		(value[i] && apply(#'call_other, value[i], data))) > prio)
	    {
	        prio = ret;
		max_ob = value[i];
	    }
	return max_ob;
    }
}

/*
FUNKTION: do_notifies
DEKLARATION: void do_notifies(int flags, string message, mixed *postfixes, mixed *obs, varargs mixed *params)
BESCHREIBUNG:
Diese Funktion kuemmert sich um den Aufruf mehrerer notify-Funktionen.

Zur Erklaerung erstmal ein Beispiel:
    
    do_notifies(C_RESORT, "feel", ({ "", "_me"}), ({who, what}));
     
ruft die Controller
    
    who->notify("feel", what, who);
    what->notify("feel_me", who, what);

und (weniger offensichtlich)

    all_environment(who)[<1]->notify("feel_here", who, what);
    
auf. Falls 'what' ein V-Item ist, wird die Funktion am zugehoerigen Objekt
aufgerufen.



Diese Funktion ruft also fuer jedes Objekt in 'obs'

    obs[i]->notify(message+postfix[i], obs..., params...);

auf. Also jedem Objekt in 'obs' ist ein String in 'postfix' zugeordnet,
welcher dem Controllernamen 'message' angehaengt wird. Ist obs[i] ein
V-Item, so wird das zugehoerige Objekt genommen. Gibt es ein Objekt obs[i]
mit "" als Postfix, so wird in seiner aeussersten Umgebung

    all_environment(obs[i])[<1]->notify(message+"_here", obs..., params...)

aufgerufen. Null-Eintraege in 'obs' werden ignoriert.

Folgende Flags (definiert in control.h) kann man angeben:
    C_RESORT	Die Argumente aus 'obs' werden so umsortiert, dass das
                zum Aufruf zugehoerige Objekt am Schluss in der Objektliste
		kommt.
    C_OMIT_OBJ	Das aufgerufene Objekt wird ganz aus der Objektliste 'obs'
		beim Aufruf weggelassen.
    C_NO_HERE	Es wird nicht der _here-Controller aufgerufen.

VERWEISE: notify, add_controller, do_forbiddens
GRUPPEN: controller
*/
void do_notifies(int flags, string message, mixed *postfixes, mixed *obs, varargs mixed *params)
{
    object here;
    for(int i=0;i<sizeof(obs);i++)
    {
	mixed ob = obs[i];
	while(mappingp(ob))
	    ob = ob["environment"];
	if(!ob)
	    continue;
	if(!objectp(ob))
	    raise_error("Ungültiges Objekt im 3. Parameter von call_notify.\n");
	apply(#'call_other, ob, "notify", message+postfixes[i],
	    ((flags&(C_RESORT|C_OMIT_OBJ))?
		(obs[0..i-1]+obs[i+1..<1]+
		    ((!(flags&C_OMIT_OBJ) && (sizeof(params) || ob!=obs[i]))?obs[i..i]:({}))):
		obs)+params);
	if(postfixes[i]=="")
	    here = ob;
    }
    if(here && !(flags&C_NO_HERE) && member(postfixes, "_here")<0)
    {
	object *env=all_environment(here);
	apply(#'call_other, sizeof(env)?env[<1]:here, "notify",
	    message+"_here", obs+params);
    }
}

/*
FUNKTION: do_forbiddens
DEKLARATION: mixed do_forbiddens(int flags, string message, mixed *postfixes, mixed *obs, varargs mixed *params)
BESCHREIBUNG:
Diese Funktion kuemmert sich um den Aufruf mehrerer forbidden-Funktionen.

Zur Erklaerung erstmal ein Beispiel:
    
    do_forbiddens(C_RESORT, "feel", ({ "", "_me"}), ({who, what}));
     
ruft nacheinander die Controller
    
    who->forbidden("feel", what, who);
    what->forbidden("feel_me", who, what);

und (weniger offensichtlich)

    all_environment(who)[<1]->forbidden("feel_here", who, what);
    
auf und bricht beim ersten Ergebnis, welches nicht 0 ist, ab und liefert
es zurueck. Falls 'what' ein V-Item ist, wird die Funktion am zugehoerigen
Objekt aufgerufen.



Diese Funktion ruft also fuer jedes Objekt in 'obs'

    obs[i]->forbidden(message+postfix[i], obs..., params...);

auf. Also jedem Objekt in 'obs' ist ein String in 'postfix' zugeordnet,
welcher dem Controllernamen 'message' angehaengt wird. Ist obs[i] ein
V-Item, so wird das zugehoerige Objekt genommen. Gibt es ein Objekt obs[i]
mit "" als Postfix, so wird in seiner aeussersten Umgebung

    all_environment(obs[i])[<1]->forbidden(message+"_here", obs..., params...)

aufgerufen. Null-Eintraege in 'obs' werden ignoriert.

Folgende Flags (definiert in control.h) kann man angeben:
    C_RESORT	  Die Argumente aus 'obs' werden so umsortiert, dass das
                  zum Aufruf zugehoerige Objekt am Schluss in der Objektliste
		  kommt.
    C_OMIT_OBJ	  Das aufgerufene Objekt wird ganz aus der Objektliste 'obs'
		  beim Aufruf weggelassen.
    C_NO_HERE	  Es wird nicht der _here-Controller aufgerufen.
Die Flags C_RESORT und C_OMIT_OBJ sind vorwiegend fuer Kompatibilitaet gedacht.

VERWEISE: forbidden, add_controller, do_notifies
GRUPPEN: controller
*/
mixed do_forbiddens(int flags, string message, mixed *postfixes, mixed *obs, varargs mixed *params)
{
    object here;
    for(int i=0;i<sizeof(obs);i++)
    {
	mixed ob = obs[i];
	while(mappingp(ob))
	    ob = ob["environment"];
	if(!ob)
	    continue;
	if(!objectp(ob))
	    raise_error("Ungültiges Objekt im 3. Parameter von call_notify.\n");
	if(postfixes[i]=="")
	    here = ob;
	ob = apply(#'call_other, ob, "forbidden", message+postfixes[i],
	    ((flags&(C_RESORT|C_OMIT_OBJ))?
		(obs[0..i-1]+obs[i+1..<1]+
		    ((!(flags&C_OMIT_OBJ) && (sizeof(params) || ob!=obs[i]))?obs[i..i]:({}))):
		obs)+params);
	if(ob)
	    return ob;
    }
    if(here && !(flags&C_NO_HERE) && member(postfixes, "_here")<0)
    {
	object *env=all_environment(here);
	return apply(#'call_other, sizeof(env)?env[<1]:here, "forbidden",
	    message+"_here", obs+params);
    }
}

