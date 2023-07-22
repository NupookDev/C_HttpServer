const videoBg = document.getElementById("vd-bg")

const playVidBg = () => {
    document.body.removeEventListener("click", playVidBg)

    videoBg.src = "assets/tank_explode.webm"

    const finishHandler = () => {
        videoBg.removeEventListener("ended", finishHandler)
        videoBg.animate(
            [
                { opacity: 1 },
                { opacity: 0 }
            ],
            {
                duration: 2000,
                fill: "forwards"
            }
        )
    }

    videoBg.addEventListener("ended", finishHandler)
    videoBg.play()
    videoBg.animate(
        [
            { opacity: 0 },
            { opacity: 1 }
        ],
        {
            duration: 4000,
            fill: "forwards"
        }
    )
}

document.body.addEventListener("click", playVidBg)