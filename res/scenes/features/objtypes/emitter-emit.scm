

(define emitterOn #t)
(define (onKey key scancode action mods)
  (if (and (= key 332) (= action 1))  ; *
    (begin
      (set! emitterOn (not emitterOn))
      (display "setting emitter state: ")
      (display emitterOn)
      (display "\n")
      (gameobj-setattr! mainobj 
        (list 
          (list "state" (if emitterOn "enabled" "disabled"))
        )
      )
    )
  )
)

(define (onMouse button action mods)
  (format #t "button is: ~a ~a\n" button action)
  (if (and (= button 0) (= action 1))
    (emit (gameobj-id mainobj))
  )
)
