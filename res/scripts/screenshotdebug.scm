(define lastTime (time-seconds))
(define number 0)

(define (screenshotpath) (string-append "./build/screenshots/screenshot_" (number->string number) ".png"))
(define (debugpath) (string-append "./build/screenshots/debug_" (number->string number) ".debug"))

(define (onFrame)
  (define currtime (time-seconds))
  (if (> (- currtime lastTime) 5)
    (begin
      (set! lastTime currtime)
      (format #t "should take screenshot now!\n")
      (screenshot (screenshotpath))
      (debuginfo "" (debugpath))
      (set! number (+ number 1))
    )
  )
)