(define lastTime (time-seconds))
(define number 0)

(define (screenshotpath) (string-append "./build/screenshots/auto_screenshot_" (number->string number) ".png"))
(define (debugpath) (string-append "./build/screenshots/auto_debug_" (number->string number) ".debug"))

(define manualnumber 0)
(define (manualscreenshotpath) (string-append "./build/screenshots/screenshot_" (number->string manualnumber) ".png"))
(define (manualdebugpath) (string-append "./build/screenshots/debug_" (number->string manualnumber) ".debug"))

(define shouldTakeAutoScreens (equal? (args "screenshot") #t))

(define (onFrame)
  (define currtime (time-seconds))
  (if (and shouldTakeAutoScreens (> (- currtime lastTime) 5))
    (begin
      (set! lastTime currtime)
      (format #t "should take screenshot now!\n")
      (screenshot (screenshotpath))
      (debuginfo "" (debugpath))
      (set! number (+ number 1))
    )
  )
)

(define (onKeyChar key)
  (if (equal? key 45)  ; - key
    (begin
      (format #t "should save!\n")
      (screenshot (manualscreenshotpath))
      (debuginfo "" (manualdebugpath))
      (set! manualnumber (+ manualnumber 1))
    )
  )
)