

(define (playAnimation)
  (define animations (gameobj-animations mainobj))
  (display (string-append "play animation: " (car animations) "\n"))
  (gameobj-playanimation mainobj (car animations))
)

(define (onKey key scancode action mods)
  (display (string-append "Key is: " (number->string key) "\n"))
  (if (equal? key 257)
    (playAnimation)
  )
)

