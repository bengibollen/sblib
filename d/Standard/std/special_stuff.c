/*
   /d/Standard/std/special_stuff.c

*/

#define WIZ_PATH "/d/Standard/doc/infowiz/"

void start_special(mixed qroom)
{
    call_other(qroom, "query_mail");
}
    
void finger_special()
{
    string file, nam;

    nam = ({string}) this_object()->query_real_name();
    file = WIZ_PATH + nam;
    if (file_size(file) >= 0)
    {
	write("--------- Special mud info on: " + capitalize(nam) + "\n");
	cat(file);
    }
}

