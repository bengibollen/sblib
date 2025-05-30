// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/description_core.c
// Description:	Dynamische Beschreibungen
// Author:	Gnomi

#pragma strong_types
#pragma save_types

#include <description_core.h>
#include <apps.h>
#include <lpctypes.h>

// Diese Funktionen sind zum Ueberlagern...
/*
FUNKTION: desc_add
DEKLARATION: protected mixed desc_add(mixed s1, mixed s2)
BESCHREIBUNG:
Diese Funktion wird bei der Generation des Beschreibungstextes aufgerufen
und fuegt zwei Teile einer Beschreibung zusammen. Man kann sie ueberlagern,
um z.B. andere Typen von Satzteilen neben Strings zu ermoeglichen.
VERWEISE: compile_desc
GRUPPEN: beschreibung
*/
protected mixed desc_add(mixed s1, mixed s2)
{
    return s1+s2;
}

/*
FUNKTION: desc_call_other_func
DEKLARATION: protected mixed desc_call_other_func(object obj, string func, mixed info, mixed* par)
BESCHREIBUNG:
Diese Funktion wird bei T_CALL_OTHER, T_FILTER_CALL_OTHER oder
T_TEXT_CALL_OTHER-Aufrufen genutzt, um die angegebene Funktion aufzurufen.
Die Standardimplementation ruft diese Funktion ueber denClosure-Container
auf. Diese Funktion kann ueberlagert werden, um den Funktionsaufruf
anderweitig zu gestalten.
VERWEISE: compile_desc, desc_call_func, desc_call_cl
GRUPPEN: beschreibung
*/
protected mixed desc_call_other_func(object obj, string func, mixed info, mixed* par)
{
    return apply(#'call_other, CLOSURE_CONTAINER, "do_call", obj, func, info, par);
}


/*
FUNKTION: desc_call_func
DEKLARATION: protected mixed desc_call_func(string func, mixed info, mixed* par)
BESCHREIBUNG:
Diese Funktion wird bei T_FUNC, T_FILTER_FUNC oder T_TEXT_FUNC-Aufrufen
genutzt, um die angegebene Funktion aufzurufen. Die Standardimplementation
ruft diese Funktion im eigenen Objekt ueber den Closure-Container auf.
Diese Funktion kann ueberlagert werden, um den Funktionsaufruf
anderweitig zu gestalten.
VERWEISE: compile_desc, desc_call_cl, desc_call_other_func
GRUPPEN: beschreibung
*/
protected mixed desc_call_func(string func, mixed info, mixed* par)
{
    return apply(#'call_other, CLOSURE_CONTAINER, "do_call", this_object(), func, info, par);
}

/*
FUNKTION: desc_call_cl
DEKLARATION: protected mixed desc_call_cl(closure cl, mixed info, varargs mixed* par)
BESCHREIBUNG:
Diese Funktion wird bei direkten Angaben von Closures im Array oder bei
T_CL, T_FILTER_CL oder T_TEXT_CL-Aufrufen genutzt, um die uebergebene
Closure aufzurufen. Die Standardimplementation ruft sie direkt mit
'info' und den Elementen aus 'par' als weitere Parameter auf.
Diese Funktion kann ueberlagert werden, um die Ausfuehrung der Closure
anderweitig zu gestalten.
VERWEISE: compile_desc, desc_call_func, desc_call_other_func
GRUPPEN: beschreibung
*/
protected mixed desc_call_cl(closure cl, mixed info, varargs mixed* par)
{
    if(get_type_info(cl, 1)==CLOSURE_IDENTIFIER)
	return funcall(cl); // Variablen vertragen keinen Parameter.

    return apply(cl, info, par);
}

/*
FUNKTION: desc_condition
DEKLARATION: protected mixed desc_condition(string name, mixed info, mixed* par)
BESCHREIBUNG:
Diese Funktion wertet durch ihren Namen angegebene Bedingungen aus.
Die Standardimplementation ruft im aktuellen Objekt die Funktion
"desc_condition_"+name mit 'info' und den Elementen aus 'par' als weitere
Parameter auf, sofern sie vorhanden ist.
Diese Funktion kann ueberlagert werden, um bestimmte Bedingungen gleich
direkt dort zu implementieren.
VERWEISE: compile_desc, desc_filter, desc_text, desc_number
GRUPPEN: beschreibung
*/
protected mixed desc_condition(string name, mixed info, mixed* par)
{
    return apply(#'call_other, this_object(), "desc_condition_"+name, info, par);
}

/*
FUNKTION: desc_filter
DEKLARATION: protected mixed desc_filter(string name, mixed info, mixed orig, mixed* par)
BESCHREIBUNG:
Diese Funktion implementiert durch Namen angebene Filter. Als dritten
Parameter erhaelt sie den zu filternden Text.
Die Standardimplementation ruft im aktuellen Objekt die Funktion
"desc_filter_"+name mit 'info', 'orig' und den Elementen aus 'par' als
weitere Parameter auf, sofern sie vorhanden ist.
Diese Funktion kann ueberlagert werden, um bestimmte Filter gleich
direkt dort zu implementieren.
VERWEISE: compile_desc, desc_condition, desc_text, desc_number
GRUPPEN: beschreibung
*/
protected mixed desc_filter(string name, mixed info, mixed orig, mixed* par)
{
    return apply(#'call_other, this_object(), "desc_filter_"+name, info, orig, par);
}

/*
FUNKTION: desc_text
DEKLARATION: protected mixed desc_text(string name, mixed info, mixed* par)
BESCHREIBUNG:
Diese Funktion generiert durch ihren Namen angegebene Texte.
Die Standardimplementation ruft im aktuellen Objekt die Funktion
"desc_text_"+name mit 'info' und den Elementen aus 'par' als weitere
Parameter auf, sofern sie vorhanden ist.
Diese Funktion kann ueberlagert werden, um bestimmte Texte gleich
direkt dort zu implementieren.
VERWEISE: compile_desc, desc_condition, desc_filter, desc_number
GRUPPEN: beschreibung
*/
protected mixed desc_text(string name, mixed info, mixed* par)
{
    return apply(#'call_other, this_object(), "desc_text_"+name, info, par);
}

/*
FUNKTION: desc_number
DEKLARATION: protected mixed desc_number(string name, mixed info, mixed* par)
BESCHREIBUNG:
Diese Funktion liefert durch ihren Namen angebene Eigenschaften zum
Vergleich mit T_GREATER, T_EQUAL & Co. zurueck.
Die Standardimplementation ruft im aktuellen Objekt die Funktion
"desc_number"+name mit 'info' und den Elementen aus 'par' als weitere
Parameter auf, sofern sie vorhanden ist.
Diese Funktion kann ueberlagert werden, um bestimmte Werte gleich
direkt dort zu implementieren.
VERWEISE: compile_desc, desc_condition, desc_filter, desc_text
GRUPPEN: beschreibung
*/
protected int desc_number(string name, mixed info, mixed* par)
{
    return apply(#'call_other, this_object(), "desc_number_"+name, info, par);
}

/*
FUNKTION: desc_compile
DEKLARATION: protected int desc_compile(string name, mixed res, mixed* par)
BESCHREIBUNG:
Diese Funktion wird waehrend des Compilierens fuer T_ATOM_COMPILE-Elemente
aufgerufen, um alternative Sprachen zu unterstuetzen. 'res' wird per
Referenz uebergeben und darin wird als Rueckgabewert ein Lambda-Ausdruck
erwartet. Die Funktion muss zudem eine der folgenden Konstanten zurueckliefern:
    T_ATOM_NOTHING:	In 'res' wurde nichts zurueckgegeben.
    T_ATOM_COND:	'res' enthaelt eine Bedingung.
    T_ATOM_FILTER:	'res' enthaelt einen Filter.
    T_ATOM_TEXT:	'res' enthaelt einen Text.
    
VERWEISE: compile_desc
GRUPPEN: beschreibung
*/
protected int desc_compile(string name, mixed res, mixed* par)
{
    return T_ATOM_NOTHING;
}

#define T_ATOM_IFBRANCH		-1
#define T_ATOM_ELSEBRANCH	-2
#define T_ATOM_NOELSE		-3

#define STACK_TYPE	0
#define STACK_RES	1

// Einen Wert auf den Compile-Stack legen, ggf. reduzieren und Ausgabe produzieren.
// Regeln nach denen reduziert wird:
// 	IFBRANCH	<= COND TEXT
//	ELSEBRANCH	<= IFBRANCH ELSE
// 	TEXT     	<= ELSEBRANCH TEXT
// 	TEXT     	<= IFBRANCH NOELSE
// 	TEXT     	<= FILTER TEXT
private void compile_stack(mixed stack, int type, mixed elem, mixed res)
{
    while(type != T_ATOM_NOTHING)
    {
	if(sizeof(stack) && stack[0][STACK_TYPE]==T_ATOM_ELSE)
	    raise_error("Misplaced T_ELSE.\n");
	
	switch(type)
	{
	    case T_ATOM_COND:
	    case T_ATOM_FILTER:
	    case T_ATOM_IFBRANCH:
		stack += ({ ({type, elem}) });
		type = T_ATOM_NOTHING;
		break;
		
	    case T_ATOM_NOELSE:
		if(sizeof(stack) && (stack[<1][STACK_TYPE]==T_ATOM_IFBRANCH))
		{
		    elem = stack[<1][STACK_RES] + ({""});
		    type = T_ATOM_TEXT;

		    stack = stack[0..<2];
		}
		else
		    type = T_ATOM_NOTHING;
		break;
	    
	    case T_ATOM_ELSE:
		if(!sizeof(stack))
		{
		    // Auf den Stack legen fuer den Fall, dass
		    // das das einzige Element im Array ist.
		    // (Anderenfalls gibt's spaeter einen Fehler.)
		    stack = ({ ({ type, elem}) });
		    type = T_ATOM_NOTHING;
		}
		else if(stack[<1][STACK_TYPE] == T_ATOM_IFBRANCH)
		{
		    stack[<1][STACK_TYPE] = T_ATOM_ELSEBRANCH;
		    type = T_ATOM_NOTHING;
		}
		else
		    raise_error("T_ELSE without condition.\n");
		break;
	
	    case T_ATOM_TEXT:
		if(!sizeof(stack))
		{
		    if(res)
			res = ({#'desc_add, res, elem});
		    else
			res = elem;
		    type = T_ATOM_NOTHING;
		}
		else
		{
		    switch(stack[<1][STACK_TYPE])
		    {
			case T_ATOM_COND:
			    elem = ({#'?, stack[<1][STACK_RES], elem});
			    type = T_ATOM_IFBRANCH;
			    break;
			    
			case T_ATOM_ELSEBRANCH:
			    elem = stack[<1][STACK_RES] + ({ elem });
			    type = T_ATOM_TEXT;
			    break;
			    
			case T_ATOM_FILTER:
			    stack[<1][STACK_RES][3] = elem;
			    elem = stack[<1][STACK_RES];
			    type = T_ATOM_TEXT;
			    break;
			    
			default:
			    printf("%O\n", stack);
			    raise_error("Inconsistent stack during compilation.\n");
		    }
		    stack = stack[0..<2];
		}
		break;
	
		
	}
    }
}
	    
// Liefert entweder T_ATOM_NOTHING (zu ignorieren),
// T_ATOM_COND, T_ATOM_FILTER, T_ATOM_TEXT oder T_ATOM_ELSE zurueck.
private int compile_arr(mixed arr, mapping tags, mixed res)
{
    if(intp(arr[0]))
    {
	switch(arr[0])
	{
	    case T_ATOM_COND:
		if(intp(arr[1]))
		{
		    closure op;
		    res = 0;
		    
		    switch(arr[1])
		    {
			case T_ATOM_FUNC:
			    res = ({#'desc_call_func, arr[2], 'info, quote(arr[3..<1])});
			    break;
			case T_ATOM_CALL_OTHER:
			    res = ({#'desc_call_other_func, arr[2], arr[3], 'info, quote(arr[4..<1])});
			    break;
			case T_ATOM_GREATER:
			    op = #'>;
			    break;
			case T_ATOM_LESSER:
			    op = #'<;
			    break;
			case T_ATOM_EQUAL:
			    op = #'==;
			    break;
			case T_ATOM_NE:
			    op = #'!=;
			    break;
			case T_ATOM_GE:
			    op = #'>=;
			    break;
			case T_ATOM_LE:
			    op = #'<=;
			    break;
		    }
		    
		    if(!res)
		    {
			if(op)
			{
			    res = ({ op, 0, arr[<1] });

			    if(intp(arr[2]))
			        switch(arr[2])
				{
				    case TN_ATOM_FUNC:
					res[1] = ({#'desc_call_func, arr[3], 'info, quote(arr[4..<2])});
					break;
				    case TN_ATOM_CALL_OTHER:
					res[1] = ({#'desc_call_other_func, arr[3], arr[4], 'info, quote(arr[5..<2])});
					break;
				    default:
					raise_error("Unknown property "+arr[2]+".\n");
				}
			    else if(stringp(arr[2]))
				res[1] = ({#'desc_number, arr[2], 'info, quote(arr[3..<1])});
			    else if(closurep(arr[2]))
				res[1] = ({#'desc_call_cl, arr[2], 'info, quote(arr[3..<1])});
			    else
				raise_error("Illegal property.\n");
			}
			else
			    raise_error("Unknown special condition "+arr[1]+".\n");
		    }
		}
		else if(stringp(arr[1]))
		    res = ({#'desc_condition, arr[1], 'info, quote(arr[2..<1])});
		else if(closurep(arr[1]))
		    res = ({#'desc_call_cl, arr[1], 'info, quote(arr[2..<1])});
		else
		    raise_error("Illegal condition.\n");
		
		return T_ATOM_COND;
		
	    case T_ATOM_FILTER:
		if(intp(arr[1]))
		    switch(arr[1])
		    {
			case T_ATOM_FUNC:
			    res = ({#'desc_call_func, arr[2], 'info, "", quote(arr[3..<1])});
			    break;
			case T_ATOM_CALL_OTHER:
			    res = ({#'desc_call_other_func, arr[2], arr[3], 'info, "", quote(arr[4..<1])});
			    break;
			default:
			    raise_error("Unknown special filter "+arr[1]+".\n");
		    }
		else if(stringp(arr[1]))
		    res = ({#'desc_filter, arr[1], 'info, "", quote(arr[2..<1])});
		else if(closurep(arr[1]))
		    res = ({#'desc_call_cl, arr[1], 'info, "", quote(arr[2..<1])});
		else
		    raise_error("Illegal filter.\n");
		
		return T_ATOM_FILTER;

	    case T_ATOM_TEXT:
		if(intp(arr[1]))
		    switch(arr[1])
		    {
			case T_ATOM_FUNC:
			    res = ({#'desc_call_func, arr[2], 'info, quote(arr[3..<1])});
			    break;
			case T_ATOM_CALL_OTHER:
			    res = ({#'desc_call_other_func, arr[2], arr[3], 'info, quote(arr[4..<1])});
			    break;
			default:
			    raise_error("Unknown special text operation "+arr[1]+".\n");
		    }
		else if(stringp(arr[1]))
		    res = ({#'desc_text, arr[1], 'info, quote(arr[2..<1])});
		else if(closurep(arr[1]))
		    res = ({#'desc_call_cl, arr[1], 'info, quote(arr[2..<1])});
		else
		    raise_error("Illegal text operation.\n");
		    
		return T_ATOM_TEXT;

	    case T_ATOM_TAG:
		if(sizeof(arr)>2)
		    m_add(tags, arr[1], arr[2]);
		else if(!member(tags, arr[1]))
		    m_add(tags, arr[1], 0);
		return T_ATOM_NOTHING;

	    case T_ATOM_COMPILE:
		if(stringp(arr[1]))
		    return desc_compile(arr[1], &res, arr[2..<1]);
		else
		    raise_error("Illegal compile element.\n");

	    case T_ATOM_ELSE:
		return T_ATOM_ELSE;
		
	    case T_ATOM_AND: // Fall through.
	    case T_ATOM_OR:
		if(arr[0]==T_ATOM_AND)
		    res = ({#'&&});
		else
		    res = ({#'||});
		    
		for(int i=1;i<sizeof(arr);i++)
		{
		    mixed condres;
		    int condtype;
			
		    condtype = compile_arr(arr[i], tags, &condres);
		    if(condtype==T_ATOM_COND)
		        res += ({ condres });
		    else if(condtype!=T_ATOM_NOTHING)
		        raise_error("Illegal array type at position "+(i-1)+
			    " in "+((arr[0]==T_ATOM_AND)?"AND":"OR")+
			    "-operation.\n");
		}
		
		if(sizeof(res)>2)
		    ;
		else if(sizeof(res)==2)
		    res=res[1];
		else if(arr[0]==T_ATOM_AND)
		    res=1;
		else
		    res=0;
		
		return T_ATOM_COND;

	    case T_ATOM_NOT:
	    {
		mixed condres;
		int condtype;
			
		condtype = compile_arr(arr[1], tags, &condres);
		if(condtype==T_ATOM_COND)
		    res = ({ #'!, condres });
		else
		    raise_error("Illegal array type in T_NOT-operation.\n");
		
		return T_ATOM_COND;
	    }

	    default:
		raise_error("Unknown array type "+arr[0]+".\n");
	}
    }
    else
    {
	mixed* stack = ({});
	res = 0;
	
	// Wir muessen die einzelnen Elemente zusammenaddieren.
	foreach(mixed elem: arr)
	{
	    mixed elemres;
	    int elemtype;
	    
	    if(stringp(elem))
	    {
		elemtype = T_ATOM_TEXT;
		elemres = elem;
	    }
	    else if(pointerp(elem))
		elemtype = compile_arr(elem, tags, &elemres);
	    else if(closurep(elem))
	    {
		elemtype = T_ATOM_TEXT;
		elemres = ({#'desc_call_cl, elem, 'info});
	    }
	    else
		raise_error("Illegal array element at position "+
		    member(arr, elem)+".\n");

	    if(elemtype == T_ATOM_NOTHING)
		continue;
	    
	    if(elemtype != T_ATOM_ELSE && sizeof(stack) && stack[<1][STACK_TYPE]==T_ATOM_IFBRANCH)
		compile_stack(&stack, T_ATOM_NOELSE, 0, &res);

	    compile_stack(&stack, elemtype, elemres, &res);
	}
	
	compile_stack(&stack, T_ATOM_NOELSE, 0, &res); // Stack leeren.
	if(!res)
	{
	    if(!sizeof(stack))
		return T_ATOM_NOTHING;
	    if(sizeof(stack)==1)
	    {
		res = stack[0][STACK_RES];
		return stack[0][STACK_TYPE];
	    }
	}
	
	if(sizeof(stack)) // Auf dem Stack liegt noch unvollstaendiges Zeug.
	    raise_error("Condition or filter without text.\n");
	
	return T_ATOM_TEXT;
    }
}

/*
FUNKTION: compile_desc
DEKLARATION: protected mixed compile_desc(mixed desc, mapping tags)
BESCHREIBUNG:
Diese Funktion wandelt eine Beschreibung mit einem im folgenden erklaerten
Aufbau in eine Closure um. Falls 'desc' ein String oder bereits eine
Closure ist, so wird diese direkt zurueckgeliefert. Ansonsten wird
die generierte Closure zurueckgegeben. In 'tags' werden die Namen aller
gefundenen T_TAG-Anweisungen gespeichert.

Eine verarbeitete Beschreibung ist im einfachsten Falle ein Array
mit einzelnen Teilen der Beschreibung. Diese Elemente koennen sein:

 - Ein String: Dieser wird so in die Beschreibung uebernommen.

 - Eine Closure: Diese wird bei Auswertung der Beschreibung (also, wenn
   sie jemand betrachtet) ausgewertet und das Ergebnis dort in die Meldung
   eingefuegt.

 - Eine Bedingung: Der nachfolgende Satzteil wird nur dann in die
   Meldung uebernommen, wenn die Bedingung wahr ist. Bedingungen
   werden durch Defines wie T_DARK (=es ist dunkel) oder
   T_VCLOCK(200000,220000) (=es ist zwischen 20 und 22 Uhr) dargestellt.
   Diese Bedingungen sind in den jeweiligen Includes (fuer Raeume room.h)
   definiert. Die spezielle Bedingung T_ELSE ist wahr, wenn die unmittelbar
   vorhergehende Bedingung falsch ist.
 
 - Ein Filter: Der Filter erhaelt den nachfolgenden Satzteil und kann
   ihn veraendern. Er wird ebenfalls durch entsprechende T-Defines
   dargestellt.
 
 - Ein vordefinierter Text: Er ist ebenfalls ein T-Define (z.B. T_DAYTIME),
   und wird beim Betrachten ausgewertet und dort eingefuegt.
 
 - Ein Array: Mit einem Array lassen sich mehrere Elemente gruppieren.

BEISPIEL:

    ({
	T_DARK, "Es ist dunkel hier, du hoerst aber Wasser plaetschern. ",
	T_ELSE,
	({
	    "Du befindest dich am Ufer eines herrlich blauen Sees. ",
	
	    T_DAY,
	    ({
	        "In der Mitte des Sees erkennst du eine Insel. ",

		T_GREATER(TN_STAT_STR, 45),
		"Ob du wohl rueberschwimmen kannst? ",
		T_ELSE,
		"Sie ist aber zu weit weg, um zu ihr zu schwimmen. ",
	    })
	})
    })

VERWEISE: compile_cond, desc_condition, desc_filter, desc_text, desc_number,
	  desc_add, desc_call_func, desc_call_other_func, desc_call_cl,
	  T_LISTE, T_ELSE, T_AND, T_OR, T_GREATER, T_FUNC, T_CL, T_CALL_OTHER,
	  T_FILTER_FUNC, T_FILTER_CL, T_FILTER_CALL_OTHER,
	  T_TEXT_FUNC, T_TEXT_CL, T_TEXT_CALL_OTHER, T_TAG
GRUPPEN: beschreibung
*/
protected mixed compile_desc(mixed desc, mapping tags)
{
    mixed l;
    
    if(!tags)
	tags = ([:1]);

    if(stringp(desc) || closurep(desc))
	return desc;

    if(!pointerp(desc))
	return 0;

    if(compile_arr(desc, tags, &l) != T_ATOM_TEXT)
	raise_error("Incomplete text.\n");

    return lambda(({'info}), l);
}

/*
FUNKTION: compile_cond
DEKLARATION: protected mixed compile_cond(mixed cond, mapping tags)
BESCHREIBUNG:
compile_cond funktioniert aehnlich wie compile_desc, wandelt aber
nur Bedingungen (z.B. T_NIGHT) in eine Closure um.
VERWEISE: compile_desc, desc_condition
GRUPPEN: beschreibung
*/
protected mixed compile_cond(mixed cond, mapping tags)
{
    mixed l;
    
    if(!tags)
	tags = ([:1]);

    if(intp(cond) || closurep(cond))
	return cond;

    if(!pointerp(cond))
	return 0;

    if(compile_arr(cond, tags, &l) != T_ATOM_COND)
	raise_error("Not a condition.\n");

    return lambda(({'info}), ({#'&&, l, 1}));
}
