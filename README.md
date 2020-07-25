# Petunjuk Penggunaan Program

# Pembagian Tugas
- Fata Nugraha (13517109) mengerjakan refactor dari fungsi dijkstra serial, fungsi untuk memparalelkan dijkstra, dan bashfile untuk kompilasi, serta pengujian pada server.
- Edward Alexander jaya (13517115) mengerjakan fungsi dijkstra serial, fungsi untuk memparalelkan dijkstra, dan fungsi file eksternal, serta pengujian pada server.

# Laporan Pengerjaan
#### Deskripsi Solusi Paralel
- Setiap proses melakukan satu atau lebih algoritma dijkstra. Jumlah algoritma dijkstra pada setiap proses ditentukan dari jumlah node (N) dan jumlah proses.
Contoh:
Pada matriks dengan jumlah node (N) = 1000 dan jumlah proses = 6, **terdapat 1 proses yang menjadi main process** dan **5 proses lainnya yang melakukan algoritma dijkstra.** Oleh karena itu, terdapat 1000 / 5 = 200 kali algoritma dijkstra yang akan dijalankan setiap proses. 
- **Proses dengan rank (id proses) = 0 menjadi main process**
- **Proses dengan rank = 1 hingga proses dengan rank = 5 menjadi proses yang menjalankan algoritma dijkstra:**
  - Proses dengan rank = 1 menjalankan algoritma dijkstra dengan node asal 0 hingga 199.
  - Proses dengan rank = 2 menjalankan algoritma dijkstra dengan node asal 200 sampai 399.
  - Proses dengan rank = 3 menjalankan algoritma dijkstra dengan node asal 400 sampai 599.
  - Proses dengan rank = 4 menjalankan algoritma dijkstra dengan node asal 600 sampai 799.
  - Proses dengan rank = 5 menjalankan algoritma dijkstra dengan node asal 800 sampai 999.
   
- **Main process menyatukan semua hasil dari algoritma dijkstra setelah proses dengan rank = 1 sampai proses dengan rank = 5 selesai menjalankan tugasnya.**

#### Analisis Solusi Paralel
Program paralel diawali dengan beberapa baris kode:

    MPI_Status Stat;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();'

`MPI_Status` nantinya digunakan untuk fungsi `MPI_Recv`. Variabel `numtasks` adalah jumlah proses yang berjalan. Variabel `rank` digunakan sebagai nomor pengidentifikasi proses yang berjalan.

`MPI_Barrier` digunakan untuk mencegah semua proses melanjutkan program. Pencegahan tersebut akan dicabut (di-unblock) jika semua proses sudah menjalankan `MPI_Barrier`. Hal ini berguna dalam sinkronisasi waktu mulai pengerjaan dijkstra.

Setelah start_time dijalankan, terdapat inisialisasi beberapa variabel:

    int jobs = N/(numtasks-1);
    long* dataRecv;
    int destinationRank = 0;
    
`int jobs` adalah variabel yang menyimpan jumlah tugas yang harus dijalankan oleh satu proses. `long* dataRecv` digunakan untuk menyimpan data yang diterima oleh `MPI_Recv`. `int destinationRank` digunakan sebagai target pada `MPI_Send`.

Selanjutnya, terdapat dua kasus pada program ini, yaitu proses dengan rank = 0 yang menjalankan program dan proses dengan rank selain 0 yang sedang menjalankan program.

Berikut adalah kode program jika rank proses = 0:

     if (!rank){
        dataRecv = (long*) malloc(sizeof(long) * N*jobs);
        while ( count < numtasks-1 ){
            MPI_Recv(dataRecv, N*jobs, MPI_LONG, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &Stat);
            printf("Received from process %d ", Stat.MPI_SOURCE);
            for (int i = 0; i < jobs; ++i) {
                for (int j = 0; j < N; ++j) {
                    graph[Stat.MPI_SOURCE * jobs - jobs + i][j] = dataRecv[i * N + j];
                }
            }
            count++;
        }
        free(dataRecv);
    }

Singkatnya, `dataRecv` diberikan alokasi sebanyak N (jumlah node) dikali dengan `jobs`. `MPI_Recv` menerima `dataRecv` dan variabel `graph` yang bertipe `**long` menyimpan jarak terpendek dari dijkstra sejumlah `jobs` (dikerjakan oleh satu proses). Variabel `graph` pada akhirnya menyimpan seluruh jarak terpendek dari setiap node ke node lain. 

Variabel `count` digunakan untuk menghitung jumlah proses yang sudah selesai menjalankan dijkstra. Jika sudah, maka kondisi `while (count < numtasks -1)` tidak terpenuhi kembali.

Berikut adalah kode program jika rank proses bukan 0:

        long *dataSend = (long*) malloc(sizeof(long*) * N * jobs);
        int count = 0;
        for (int i = rank*jobs-jobs; i < rank*jobs; ++i)
        {   
            dijkstra(graph, i);
            for (int j = 0; j < N; j++) {
                dataSend[count * N + j] = graph[i][j];
            }
            count++;
        }
        MPI_Send(dataSend, N*jobs, MPI_LONG, destinationRank, tag, MPI_COMM_WORLD);
        free(dataSend);

Singkatnya, setiap proses menjalankan fungsi dijkstra. Fungsi dijkstra mengubah nilai dari `graph[<node yang dikerjakan>]` menjadi jarak terpendek dari node yang dikerjakan ke semua node. 

`MPI_Send` digunakan untuk mengirim data berupa jarak terpendek dari node yang dikerjakan ke semua node dan diterima oleh `MPI_Recv` pada proses dengan rank = 0.

Sebenarnya terdapat solusi yang lebih cepat, yaitu dengan meng-assign setiap process untuk mengambil task lain ketika process tersebut sudah selesai, bukan dengan membagi task di awal. Hal ini mungkin dengan menggunakan `pragma omp task`, namun use casenya pada shared memory model.
Selain itu, alokasi matriks seharusnya dilakukan hanya dilakukan satu kali (tidak secara berulang-ulang) agar array menjadi kontigu. Dengan kata lain, seharusnya representasi matriks adalah array 1 dimensi, bukan 2 dimensi.

#### Jumlah Thread yang Digunakan
- Terdapat enam proses yang digunakan pada program ini.
- IP server dimulai dari 167.205.35.150 sampai 167.205.35.155. Jika digunakan command `lscpu`, terlihat bahwa setiap server (artinya setiap IP) memiliki jumlah CPU core sebanyak satu. Dengan alasan tersebut, terdapat enam proses yang bisa dijalankan.
#### Pengukuran Kinerja untuk tiap Kasus Uji
Berikut adalah hasil pengujian yang dikerjakan pada server 13517115@167.205.35.150:
- **N = 100**

  | Tipe | Percobaan 1 | Percobaan 2 | Percobaan 3 |
  |---|--- |---|---|
  | Serial   | 12495 µs   | 13546 µs    | 13202 µs|
  | Paralel | 11397 µs | 9392 µs | 11365 µs|

- **N = 500**

  | Tipe  |  Percobaan 1 | Percobaan 2  | Percobaan 3  |
  |---|---|---|---|
  | Serial |  1637802 µs  |  1531207 µs |  1663832 µs |
  | Paralel  |  1567543 µs | 1368517 µs  |  1004626 µs |
- **N = 1000**

  | Tipe  |  Percobaan 1 | Percobaan 2  | Percobaan 3  |
  |---|---|---|---|
  | Serial | 13642516 µs | 14617350 µs | 14184775 µs  |
  | Paralel  | 4268241 µs  |  5128194 µs | 4290042 µs|
- **N = 3000**

  | Tipe  |  Percobaan 1 | Percobaan 2  | Percobaan 3  |
  |---|---|---|---|
  | Serial | 480282249 µs  |  481598290 µs |  396111461 µs|
  | Paralel  | 278312732 µs | 308372772 µs |  198770362 µs|

#### Analisis Perbandingan Kinerja Serial dan Paralel
- Pada program serial, proses yang dijalankan hanya satu. Pada program paralel, terdapat enam proses yang dijalankan. Seharusnya kecepatan program paralel adalah `6-1 = 5` kali lebih cepat daripada program serial. Namun pada program paralel, semua proses harus disinkronisasi ( atau join ) dan hal tersebut memakan waktu cukup besar. Secara detail, jika satu proses sudah selesai menjalankan tugasnya, proses tersebut akan mengirimkan `MPI_RECV` dan proses dengan rank = 0 akan menerima `MPI_SEND`. Setelah itu, proses tersebut diblok untuk melanjutkan program oleh `MPI_BARRIER` sampai semua proses lainnya selesai dijalankan. Karena waktu pemblokiran tersebut bisa bervariasi tergantung proses, maka faktor ini juga harus dipertimbangkan.
- `MPI_SEND` dan `MPI_RECV` memerlukan waktu yang proporsional dengan besar message yang dikirimkan. Artinya, semakin besar message yang dikirimkan, semakin besar overhead yang diperlukan. Sebagai contoh, pada pengujian dengan kasus uji N = 3000, satu proses mengerjakan `3000/5 = 600` task. Artinya, terdapat `3000 x 600 = 1.800.000` elemen yang akan dikirimkan satu proses kepada MPI_SEND. Satu elemen tersebut bertipe `long (8 bytes)`. Sehingga satu proses harus mengirimkan `1.800.000 X 8 bytes = 14.400.000 B = 14 MB` kepada proses dengan rank = 0. Berbeda dengan pada pengujian dengan kasus uji N = 1000, terdapat hanya `1000 / 5 * 1000 * 8 bytes = 1.6 MB` yang dikirimkan pada proses dengan rank = 0. Oleh karena itu, perbandingan waktu serial dan paralel dengan N = 1000 lebih baik dibandingkan dengan perbandingan waktu serial dan paralel dengan N = 3000.
- Pada proses pengujian dengan N = 100 dan N = 500, perbandingan waktu serial dan paralel mirip. Hal ini dikarenakan nilai N yang kecil, sehingga overhead untuk menggabungkan 6 proses dan `MPI_SEND & MPI_RECV` memperlambat program paralel. Untuk N = 1000 dan N = 3000, perbandingan waktu serial dan paralel signifikan karena nilai N yang besar dapat diimbangi dengan jumlah proses yang besar meskipun terdapat overhead.



