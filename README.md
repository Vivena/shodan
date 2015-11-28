# shodan

TTTFS

/ ! \ faire un make clean avant de push quelque chose

TODO LIST:

    A VERIFIER --> 1-faire la struct block 
    (pas de int dans la structure block, elle doit faire pile 1024 char si j'ai bien compris)

    2-faire la fonction little-endian (done)

    3-gestion du rea/write 
        3.1-read_physical (à verif)
        3.2-write_physical (done)
        3.3-write (à verif)
        3.4-read 
            3.4.1-implementer un systeme de cache
            3.4.2-finir read

    4-faire la struct DD-header

    5-faire la struct descriptor-block

    6-faire la struct file-table

    7-faire la struct Data-block

    8-créer les macro demandées 