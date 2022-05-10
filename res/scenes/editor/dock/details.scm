(define (onObjSelected gameobj color)
  (if (equal? (gameobj-id gameobj) (gameobj-id mainobj))
    (sendnotify "dock-self-remove" (number->string (gameobj-id mainobj)))
  )
)

;; todo remove - no items in this layout, should require this 
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_right" )))
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_left" )))
