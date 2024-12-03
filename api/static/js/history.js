const ctx = document.getElementById('chart');
    const chartTekanan = document.getElementById('chartTekanan');
    const chartSuhu= document.getElementById('chartSuhu');
    const chartTegangan= document.getElementById('chartTegangan');
    const chartKecepatan= document.getElementById('chartKecepatan');
    const chartKelembapan = document.getElementById('chartKelembapan');

    async function fetchData(){
        let data = await fetch('/dump');

        let res = await data.json();
        res = res.data;
        let datas = [];
        let suhu = [];
        let kelembaban = [];
        let tegangan = [];
        let tekanan  = [];
        let labels = [];
        let idx = 0;
        for(let key in res){
            datas.push(res[key].windspeed);
            suhu.push(res[key].suhu);
            tekanan.push(res[key].tekanan);
            tegangan.push(res[key].tegangan);
            kelembaban.push(res[key].kelembaban);
            labels.push(idx);
            idx++;
        }
        console.log(res);
        // new Chart(ctx, {
        //     type: 'line',
        //     data: {
        //       labels: labels,
        //       datasets: [{
        //         label: 'Kecepatan Angin',
        //         data: datas,
        //         borderWidth: 1
        //       },
        //       {
        //         label: 'Data suhu',
        //         data: suhu,
        //         borderWidth: 1
        //       },
        //       {
        //         label: 'Data Kelembaban',
        //         data: kelembaban,
        //         borderWidth: 1
        //       },
        //       {
        //         label: 'Data Tegangan',
        //         data: tegangan,
        //         borderWidth: 1
        //       }]
        //     },
        //     options: {
        //       scales: {
        //         y: {
        //           beginAtZero: true
        //         }
        //       },
        //       elements : {
        //         point :{
        //             pointStyle : false
        //         }
        //       }
        //     }
        //   });


          new Chart(chartTekanan, {
            type: 'line',
            data: {
              labels: labels,
              datasets: [{
                label: 'Tekanan Udara',
                data: tekanan,
                borderWidth: 1,
                borderColor: '#00ff22',
                backgroundColor: '#00ff22',
              }]
            },
            options: {
              scales: {
                y: {
                    suggestedMin: Math.min(tekanan),
                    suggestedMax: Math.max(tekanan)
                }
              },
              elements : {
                point :{
                    pointStyle : false
                }
              }
            }
          });

          new Chart(chartSuhu, {
            type: 'line',
            data: {
              labels: labels,
              datasets: [{
                label: 'Suhu Udara',
                data: suhu,
                borderWidth: 1,
                borderColor: '#fcdd0f',
                backgroundColor: '#fcdd0f',
              }]
            },
            options: {
              scales: {
                y: {
                    suggestedMin: Math.min(suhu),
                    suggestedMax: Math.max(suhu)
                }
              },
              elements : {
                point :{
                    pointStyle : false
                }
              }
            }
          });

          new Chart(chartTegangan, {
            type: 'line',
            data: {
              labels: labels,
              datasets: [{
                label: 'Tegangan',
                data: tegangan,
                borderWidth: 1
              }]
            },
            options: {
              scales: {
                y: {
                    suggestedMin: Math.min(tegangan),
                    suggestedMax: Math.max(tegangan)
                }
              },
              elements : {
                point :{
                    pointStyle : false
                }
              }
            }
          });

          new Chart(chartKecepatan, {
            type: 'line',
            data: {
              labels: labels,
              datasets: [{
                label: 'Kecepatan Udara',
                data: datas,
                borderWidth: 1,
                borderColor: '#7128ad',
                backgroundColor: '#7128ad',
              }]
            },
            options: {
              scales: {
                y: {
                    suggestedMin: Math.min(datas),
                    suggestedMax: Math.max(datas)
                }
              },
              elements : {
                point :{
                    pointStyle : false
                }
              }
            }
          });

          new Chart(chartKelembapan, {
            type: 'line',
            data: {
              labels: labels,
              datasets: [{
                label: 'Kelembapan Udara',
                data: kelembaban,
                borderWidth: 1,
                borderColor: '#ff426b',
                backgroundColor: '#ff426b',
              }]
            },
            options: {
              scales: {
                y: {
                    suggestedMin: Math.min(kelembaban),
                    suggestedMax: Math.max(kelembaban)
                }
              },
              elements : {
                point :{
                    pointStyle : false
                }
              }
            }
          });
    }

    fetchData();