

(define (onKey key scancode action mods)
  (if (and (= key 257) (= action 1))
    (mk-obj-attr "object-name" '(
      ("position" (10 2 0))
      ("scale"    (1 1  1))
      ("mesh"     "../gameresources/weapons/upistol.obj")
      ("extrastr" "cool")
      ("extraint" 2)
      ("net" "sync")
    ))
  )
)



