let percent = 0;
let number = document.getElementById("number");
number.innerHTML = percent + "%";
percent++;
setInterval(() => {
  if (percent <= 65) {
    number.innerHTML = percent + "%";
    percent++;
  } else {
    clearInterval();
  }
}, 2000 / 65); 