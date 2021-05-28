

(define emitterOn #t)
(define (onKey key scancode action mods)
  (if (and (= key 46) (= action 1))
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
  (if (and (= key 47) (= action 1))
    (emit (gameobj-id mainobj))
  )
)

