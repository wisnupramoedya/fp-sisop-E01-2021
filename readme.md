Run dari `cd fp-sisop-E01-2021`:
```
# db
cd database
gcc oursql.c -o oursql
./oursql

# client
cd client
gcc oursql_client.c -o oursql_client
# root
sudo ./oursql_client
# no root
./oursql_client -u jack -p jack123
```

Struktur DB:
```
final
    /fp-sisop-E01-2021
        /client
            /oursql_client.c
            /oursql_dump.c
        /database
            /databases
            /list_user_pass.csv
            /nama_database
                /list_user_db.csv
                /nama_table.csv
            /oursql.c
            /settingan_cron_backup.tab
```

Format list_user_pass.csv => menyimpan id, username, dan password
```
IDuser1,user1,pass1
IDuser2,user2,pass2
```

Format list_user_db.csv => menyimpan id dari username yang punya hak
```
IDuser2
IDuser4
```

Format nama_table.csv
```
t=kolom1:int,kolom2:string,kolom3:int
record1_column1,record1_column2,record1_column3
record2_column1,record2_column2,record2_column3
record3_column1,record3_column2,record3_column3
```

Format db:
Jika root, db server akan menghasilkan:
`login root`
Jika user + password, db server akan menghasilkan:
`login user`

Model tampilan
```
# SELECT kolom1 FROM tabel1
result:
record1_column1
```
