/*
   /d/Standard/std/mail_stuff.c

   This handles telling the player logging in information about what mail
   exist. Also tell extra information when the 'finger' command is used.

*/

void start_mail(mixed qroom)
{
    call_other(qroom, "query_mail");
}

void finger_mail()
{
}

