 /*
  * Prototypes
  */
public void create_living();
public void reset_living();
public int query_base_stat(int stat);
varargs static void update_acc_exp(int exp, int taxfree);
public string query_real_name();
public mixed query_learn_pref(int stat);
public int query_stat(int stat);
public void update_stat(int stat);
public void attack_object(object ob);
static void start_heart();
public varargs string query_Art_name(object pobj);
public void stop_fight(mixed elist);
public string query_nonmet_name();
static nomask void move_all_to(object dest);
public object *query_team();
nomask int stat_to_exp(int stat);
int query_tell_active();
string query_align_text();
public void check_last_stats();
