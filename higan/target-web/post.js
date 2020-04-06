var LAST_GAME = 'last-game';
var button = document.createElement("button");
var select = document.createElement("select");

Module.onRuntimeInitialized = () => Module.init();

fetch('/games.json')
    .then((response) => response.json())
    .then((games) => {
        games.forEach((link) => {
            var option = document.createElement("option");
            option.innerHTML = link;
            if (link === "") {
                option.disabled = true;
            } else {
                option.value = `/games/${link}`;
            }
            select.appendChild(option);
        });

        var lastGame = localStorage.getItem(LAST_GAME);
        if (lastGame) {
            select.value = lastGame;
        }

        button.addEventListener("click", () => {
            if (select.value == "") {
                return;
            }

            Module.load(select.value, () => {
                localStorage.setItem(LAST_GAME, select.value);
                Module.start();
            });
        });

        button.innerHTML = "start game";
        document.getElementById("controls").appendChild(select);
        document.getElementById("controls").appendChild(button);
    });