
(define (onKey key scancode action mods)
  (if (equal? key 257)
    (gameobj-playanimation mainobj (string-append "Armature|" (gameobj-name mainobj)))
  )
)

