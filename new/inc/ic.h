typedef struct
{	
	char rid          [10 + 1]; //9F06
	char rid_index    [2 + 1];  //9F22
	char valid        [8 + 1];	//DF05
	char hash_flag    [2 + 1];	//DF06
	char pubkey_flag  [2 + 1];	//DF07
	char pubkey       [512 + 1];//DF02
	char pubkey_index [6 + 1];	//DF04
	char pubkey_check [128 + 1];//DF03
}pub_key_def;

typedef struct
{
	char aid            [38 + 1];
	char asi            [12 + 1];
	char app_ver        [10 + 1];
	char tac_default    [16 + 1];
	char tac_online     [16 + 1];
	char tac_refuse     [16 + 1];
	char lowest         [14 + 1];
	char random_val     [14 + 1];
	char random_max_per [8 + 1];
	char rabdom_per     [8 + 1];
	char ddol           [38 + 1];
	char pin            [8 + 1];
	char limit9f7b      [18 + 1];
	char limitdf19      [18 + 1];
	char limitdf20      [18 + 1];
	char limitdf21      [18 + 1];
}ic_para_def; 