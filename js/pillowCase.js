angular.module('myModule', ['chart.js']);
var currentIndex = 1;
var link = "https://script.google.com/macros/s/AKfycby0Z_QxgA2KyqQr8QStRj66M6V6XLiLlYhMgfs3eiJCxzvjZZ3Y/exec?rowNo=";

var currentLink = link.concat(currentIndex);

var time = [];
var xMovement = [];
var soundRecorder = [];

setInterval(function(){//runs every 1 second
    $.get(currentLink,function(data){//gets data from the link^
        if(data[0].toString()==null){//break out of the loop and reinstantiate it, or a continue
            //feature would be nice.
        }
        time.push(data[0].toString());
        xMovement.push(data[1].toString());
        soundRecorder.push(data[2].toString());
    });
    alert(time);
},1000);

var data = {
    labels: ["January", "February", "March", "April", "May", "June", "July"],
    datasets: [
        {
            label: "My First dataset",
            fillColor: "rgba(220,220,220,0.2)",
            strokeColor: "rgba(220,220,220,1)",
            pointColor: "rgba(220,220,220,1)",
            pointStrokeColor: "#0f0",
            pointHighlightFill: "#ff8",
            pointHighlightStroke: "rgba(220,220,220,1)",
            data: [65, 59, 80, 81, 56, 55, 40]
        }
    ]
};

var myLine = new Chart(document.getElementById("canvas").getContext("2d")).Line(data)
