import React from 'react'
import clsx from 'clsx'

const variants = {
  success: 'badge-success',
  error: 'badge-error',
  warning: 'badge-warning',
  primary: 'bg-primary-100 dark:bg-primary-900/30 text-primary-800 dark:text-primary-200',
  secondary: 'bg-gray-100 dark:bg-gray-800 text-gray-800 dark:text-gray-200',
}

export function Badge({ children, variant = 'secondary', className, icon: Icon, ...props }) {
  return (
    <span 
      className={clsx(
        'badge',
        variants[variant],
        className
      )}
      {...props}
    >
      {Icon && <Icon className="w-3 h-3 mr-1" />}
      {children}
    </span>
  )
}