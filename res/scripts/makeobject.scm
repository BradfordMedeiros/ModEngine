

(define (onKey key scancode action mods)
  (if (and (= key 257) (= action 1))
    (mk-obj-attr "object-name" '(
      ("position" (0 10 0))
      ("scale"    (1 1  1))
      ("mesh"     "./res/models/box/box.obj")
      ("extrastr" "cool")
      ("extraint" 2)
    ))
  )
)


